#include "pk_to_xchg_converter.hpp"

#include "step/translator_step.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_maindoc.hpp"
#include "xchg_component.hpp"
#include "xchg_node.hpp"
#include "util/xchg_kernel_utils.hpp"
#include "topology/xchg_body.hpp"
#include "topology/xchg_lump.hpp"
#include "topology/xchg_shell.hpp"
#include "topology/xchg_face.hpp"
#include "topology/xchg_loop.hpp"
#include "topology/xchg_coedge.hpp"
#include "topology/xchg_edge.hpp"
#include "topology/xchg_vertex.hpp"

#include "xchg_topo_compare.hpp"

#include <cstdio>
#include <string>
#include <set>
#include <map>

static bool TransmitBodyToXT(PK_BODY_t body, const char* output_path)
{
    PK_PART_transmit_o_t transmit_opts;
    PK_PART_transmit_o_m(transmit_opts);
    transmit_opts.transmit_format = PK_transmit_format_text_c;

    PK_BODY_t parts[] = { body };
    PK_ERROR_code_t err = PK_PART_transmit(1, parts, output_path, &transmit_opts);
    if (err != PK_ERROR_no_errors) {
        fprintf(stderr, "[Error] PK_PART_transmit failed: %d\n", err);
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    // Calculate project root directory
    std::string exe_dir = argv[0];
    for (char& c : exe_dir) if (c == '\\') c = '/';
    auto last_slash = exe_dir.rfind('/');
    std::string base = (last_slash != std::string::npos) ? exe_dir.substr(0, last_slash + 1) : "./";
    base += "../../../";

    // Use command line argument for STEP file path
    std::string step_path;
    if (argc > 1) {
        step_path = argv[1];
    } else {
        // Default to cube214.step
        step_path = base + "resource/hollow_cube.step";
    }

    // Extract filename stem for output path
    // PK_PART_transmit will automatically append .xmt_txt
    auto name_start = step_path.rfind('/');
    std::string stem = (name_start != std::string::npos)
                       ? step_path.substr(name_start + 1) : step_path;
    auto ext_pos = stem.rfind('.');
    if (ext_pos != std::string::npos)
        stem = stem.substr(0, ext_pos);
    std::string xt_path = base + "xt/" + stem + "_roundtrip";

    // 1. Start Parasolid session
    Xchg_Int32 pk_err = StartSession();
    if (pk_err != 0) {
        fprintf(stderr, "[Error] StartSession failed: %d\n", pk_err);
        return 1;
    }
    printf("[Info] Parasolid session started.\n");

    // 2. Read STEP file -> Xchg_MainDoc
    AMXT_STP_CTX_context_p ctx = nullptr;
    AMXT_STP_ERROR_code_t stp_err = AMXT_STP_CTX_create(&ctx);
    if (stp_err != AMXT_STP_ERROR_no_errors) {
        fprintf(stderr, "[Error] AMXT_STP_CTX_create failed: %d\n", stp_err);
        StopSession();
        return 1;
    }

    AMXT_STP_read_o_t read_opts;
    AMXT_STP_read_o_m(read_opts);

    Xchg_MainDocPtr main_doc = Xchg_MainDoc::Create();
    stp_err = AMXT_STP_read(ctx, step_path.c_str(), &read_opts, &main_doc);
    if (stp_err != AMXT_STP_ERROR_no_errors) {
        size_t err_instance = 0;
        const char* err_msg = nullptr;
        AMXT_STP_ERROR_ask_message(ctx, &err_instance, &err_msg);
        fprintf(stderr, "[Error] AMXT_STP_read failed: %d, entity #%zu: %s\n",
                stp_err, err_instance, err_msg ? err_msg : "unknown");
        AMXT_STP_CTX_destory(ctx);
        StopSession();
        return 1;
    }
    printf("[Info] STEP file read successfully: %s\n", step_path.c_str());

    // 3. Get root component, iterate nodes to find Body nodes
    const Xchg_ComponentPtr& root = main_doc->RootComponent();
    if (!root) {
        fprintf(stderr, "[Error] MainDoc has no root component.\n");
        AMXT_STP_CTX_destory(ctx);
        StopSession();
        return 1;
    }

    Xchg_Size_t num_nodes = root->GetNumNodes();
    printf("[Info] Root component has %zu nodes.\n", num_nodes);

    std::vector<PK_BODY_t> pk_bodies;

    for (Xchg_Size_t i = 0; i < num_nodes; ++i) {
        Xchg_NodePtr node = root->GetNodeByIndex(i);
        if (!node || node->GetNodeType() != Xchg_Node::BodyType)
            continue;

        Xchg_BodyPtr xchg_body = node->GetBodyPtr();
        if (!xchg_body) {
            fprintf(stderr, "[Warn] Node %zu is BodyType but GetBodyPtr() returned null.\n", i);
            continue;
        }

        printf("[Info] Found Body node at index %zu.\n", i);

        // 4. Convert Xchg_Body -> PK_BODY
        Xchg_Int32 convert_err = node->ConvertToPKBody();
        if (convert_err != 0) {
            fprintf(stderr, "[Error] ConvertToPKBody failed: %d\n", convert_err);
            continue;
        }
        Xchg_Int32 pk_body_tag = node->GetParasolidBody();
        if (pk_body_tag == 0) {
            fprintf(stderr, "[Error] GetParasolidBody returned null.\n");
            continue;
        }
        printf("[Info] Converted to PK_BODY tag: %d\n", pk_body_tag);

        // 5. PK_BODY -> Xchg_Body (our converter)
        PK_BODY_t original_pk_body = static_cast<PK_BODY_t>(pk_body_tag);

        PKToXchgConverter converter;
        converter.SetLogCallback([](const std::string& msg) {
            fprintf(stderr, "[Converter] %s\n", msg.c_str());
        });

        Xchg_BodyPtr roundtrip_xchg_body;
        STEPExport_ErrorCode rc = converter.Convert(original_pk_body, &roundtrip_xchg_body);
        if (rc != STEP_OK || !roundtrip_xchg_body) {
            fprintf(stderr, "[Error] PKToXchgConverter::Convert failed: %d\n", rc);
            continue;
        }
        printf("[Info] PK_BODY -> Xchg_Body succeeded.\n");

#if 0
         // Dump both bodies for human-friendly comparison
        XchgTopoCompare::DumpBody(xchg_body, "STEP->Xchg (REF)");
        XchgTopoCompare::DumpBody(roundtrip_xchg_body, "PK->Xchg (OURS)");
#endif

        // 6. Xchg_Body -> PK_BODY roundtrip
        Xchg_MainDocPtr rt_doc = Xchg_MainDoc::Create();
        Xchg_ComponentPtr rt_comp = rt_doc->CreateComponent(
            L"roundtrip", L"roundtrip", Xchg_Component::ComponentInternal);
        rt_doc->SetRootComponent(rt_comp);
        Xchg_NodePtr rt_node = rt_comp->CreateBodyNode(L"body", roundtrip_xchg_body, 0);

        Xchg_Int32 rt_err = rt_node->ConvertToPKBody();
        if (rt_err != 0) {
            fprintf(stderr, "[Error] Roundtrip ConvertToPKBody failed: %d\n", rt_err);
            continue;
        }
        Xchg_Int32 rt_pk_body = rt_node->GetParasolidBody();
        if (rt_pk_body == 0) {
            fprintf(stderr, "[Error] Roundtrip GetParasolidBody returned null.\n");
            continue;
        }
        printf("[Info] Xchg_Body -> PK_BODY roundtrip succeeded, new tag: %d\n", rt_pk_body);

        pk_bodies.push_back(static_cast<PK_BODY_t>(rt_pk_body));
    }

    int body_count = pk_bodies.size();

    if (body_count == 0) {
        fprintf(stderr, "[Warn] No body nodes found in root component.\n");
    } else {
        printf("[Info] Total %d bodies converted.\n", body_count);

        // Export all bodies to single xt file
        PK_PART_transmit_o_t transmit_opts;
        PK_PART_transmit_o_m(transmit_opts);
        transmit_opts.transmit_format = PK_transmit_format_text_c;

        PK_ERROR_code_t err = PK_PART_transmit(body_count, pk_bodies.data(), xt_path.c_str(), &transmit_opts);
        if (err != PK_ERROR_no_errors) {
            fprintf(stderr, "[Error] PK_PART_transmit failed: %d\n", err);
        } else {
            printf("[Info] Written %d bodies to: %s.x_t\n", body_count, xt_path.c_str());
        }
    }

    // 7. Cleanup
    AMXT_STP_CTX_destory(ctx);
    StopSession();
    printf("[Info] Done.\n");
    return 0;
}
