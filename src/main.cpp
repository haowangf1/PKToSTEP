#include "pk_to_xchg_converter.hpp"

#include "step/translator_step.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_maindoc.hpp"
#include "xchg_component.hpp"
#include "xchg_node.hpp"
#include "util/xchg_kernel_utils.hpp"

#include <cstdio>
#include <string>

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
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <input.step> [output.x_t]\n", argv[0]);
        return 1;
    }

    const char* step_path = argv[1];
    std::string xt_path;
    if (argc >= 3) {
        xt_path = argv[2];
    } else {
        xt_path = std::string(step_path) + ".x_t";
    }

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
    stp_err = AMXT_STP_read(ctx, step_path, &read_opts, &main_doc);
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
    printf("[Info] STEP file read successfully: %s\n", step_path);

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

    int body_count = 0;
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

        // 4. Convert Xchg_Body -> PK_BODY via Xchg_Node::ConvertToPKBody
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

        // 5. Transmit PK_BODY to .x_t file
        std::string out = xt_path;
        if (body_count > 0) {
            out = xt_path + "." + std::to_string(body_count);
        }

        if (TransmitBodyToXT(static_cast<PK_BODY_t>(pk_body_tag), out.c_str())) {
            printf("[Info] Written to: %s\n", out.c_str());
        }
        ++body_count;
    }

    if (body_count == 0) {
        fprintf(stderr, "[Warn] No body nodes found in root component.\n");
    } else {
        printf("[Info] Total %d bodies converted.\n", body_count);
    }

    // 6. Cleanup
    AMXT_STP_CTX_destory(ctx);
    StopSession();
    printf("[Info] Done.\n");
    return 0;
}
