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

#include "geom/xchg_point.hpp"

#include <cstdio>
#include <string>
#include <set>
#include <map>

static void PrintVertexCoord(const Xchg_VertexPtr& v)
{
    if (!v) { printf("(null)"); return; }
    Xchg_PointPtr pt = v->GetGeom();
    if (!pt) { printf("v%lld(?)", (long long)v->GetTopoID()); return; }
    Xchg_Double64 x, y, z;
    pt->GetCoordinates(x, y, z);
    printf("v%lld(%.2f,%.2f,%.2f)", (long long)v->GetTopoID(), x, y, z);
}

static void DumpXchgBodyTopo(const Xchg_BodyPtr& body, const char* label)
{
    printf("\n===== Xchg_Body Topo: %s =====\n", label);
    Xchg_Size_t nLumps = body->GetNumLumps();
    printf("Lumps: %zu\n", nLumps);

    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        Xchg_Size_t nShells = lump->GetNumShells();
        printf("Lump[%zu]: shells=%zu\n", li, nShells);

        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);
            Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
            printf("  Shell[%zu]: faces=%zu closed=%d outer=%d\n",
                   si, nFaces, (int)shell->IsClosed(), (int)shell->IsOuter());

            for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                Xchg_FacePtr face;
                Xchg_bool faceOrient;
                shell->GetFace(fi, face, faceOrient);
                Xchg_Size_t nLoops = face->GetNumLoops();
                printf("    Face[%zu]: shellOrient=%d loops=%zu\n", fi, (int)faceOrient, nLoops);

                for (Xchg_Size_t lli = 0; lli < nLoops; ++lli) {
                    Xchg_LoopPtr loop;
                    face->GetLoop(lli, loop);
                    Xchg_Size_t nCoedges = loop->GetNumCoedges();
                    printf("      Loop[%zu]: outer=%d orient=%d coedges=%zu\n",
                           lli, (int)loop->IsOuter(), (int)loop->GetOrientation(), nCoedges);

                    for (Xchg_Size_t ci = 0; ci < nCoedges; ++ci) {
                        Xchg_CoedgePtr coedge;
                        Xchg_bool coedgeOrient;
                        loop->GetCoedge(ci, coedge, coedgeOrient);

                        Xchg_EdgePtr edge;
                        coedge->GetEdge(edge);

                        Xchg_bool sameSense = XCHG_TRUE;
                        Xchg_VertexPtr sv, ev;
                        if (edge) {
                            sameSense = edge->GetSameSense();
                            edge->GetStartVertex(sv);
                            edge->GetEndVertex(ev);
                        }

                        printf("        CE[%zu]: inLoop=%d ceOrient=%d sameSense=%d ",
                               ci, (int)coedgeOrient, (int)coedge->GetOrientation(), (int)sameSense);
                        PrintVertexCoord(sv);
                        printf(" -> ");
                        PrintVertexCoord(ev);
                        printf("\n");
                    }
                }
            }
        }
    }
    printf("===== End: %s =====\n\n", label);
}

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
        fprintf(stderr, "Usage: %s <input.step> [output_basename]\n", argv[0]);
        return 1;
    }

    const char* step_path = argv[1];
    std::string xt_path;
    if (argc >= 3) {
        xt_path = argv[2];
    } else {
        // PK_PART_transmit 会自动追加 .xmt_txt 后缀
        xt_path = std::string(step_path) + "_roundtrip";
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
        DumpXchgBodyTopo(xchg_body, "STEP->Xchg (reference)");

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

        // 5. PK_BODY -> Xchg_Body (our converter) -> PK_BODY -> .x_t roundtrip
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
        DumpXchgBodyTopo(roundtrip_xchg_body, "PK->Xchg (ours)");

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

        // 6. Transmit roundtrip PK_BODY to .x_t file
        std::string out = xt_path;
        if (body_count > 0) {
            out = xt_path + "." + std::to_string(body_count);
        }

        if (TransmitBodyToXT(static_cast<PK_BODY_t>(rt_pk_body), out.c_str())) {
            printf("[Info] Written to: %s\n", out.c_str());
        }
        ++body_count;
    }

    if (body_count == 0) {
        fprintf(stderr, "[Warn] No body nodes found in root component.\n");
    } else {
        printf("[Info] Total %d bodies converted.\n", body_count);
    }

    // 7. Cleanup
    AMXT_STP_CTX_destory(ctx);
    StopSession();
    printf("[Info] Done.\n");
    return 0;
}
