#include "step/translator_step.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_maindoc.hpp"
#include "xchg_component.hpp"
#include "xchg_node.hpp"
#include "util/xchg_kernel_utils.hpp"
#include "topology/xchg_body.hpp"

#include <parasolid_kernel.h>

#include "../include/export_step.hpp"
#include <cstdio>
#include <string>
#include <vector>

void Export_step(Xchg_MainDocPtr* mainDoc, const std::string& input_step_path)
{
    // 从输入路径提取文件名（不含扩展名）
    auto name_start = input_step_path.rfind('/');
    if (name_start == std::string::npos) {
        name_start = input_step_path.rfind('\\');
    }
    std::string filename = (name_start != std::string::npos)
                          ? input_step_path.substr(name_start + 1) : input_step_path;

    // 去掉扩展名
    auto ext_pos = filename.rfind('.');
    std::string stem = (ext_pos != std::string::npos) ? filename.substr(0, ext_pos) : filename;

    // 生成输出文件名：原文件名 + _export.step
    std::string output_path = stem + "_export.step";

    printf("[Info] Exporting MainDoc to: %s\n", output_path.c_str());

    AMXT_STP_export_o_t options;
    AMXT_STP_export_o_m(options);

    AMXT_STP_ERROR_code_t err = AMXT_STP_export(mainDoc, &options, output_path);
    if (err != AMXT_STP_ERROR_no_errors) {
        fprintf(stderr, "[Error] AMXT_STP_export failed: %d\n", err);
    } else {
        printf("[Info] STEP file exported successfully: %s\n", output_path.c_str());
    }
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
        step_path = base + "resource/hollow_cube.step";
    }

    // Extract filename stem for output path
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

    // Export the MainDoc to STEP file
    Export_step(&main_doc, step_path);

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

    // Step 1: STEP -> Xchg -> PK，收集所有 PK_BODY
    // 同时创建一个新 MainDoc，存储这些 PK_BODY，用于后续 PK->X->PK 往返测试
    Xchg_MainDocPtr pk_doc = Xchg_MainDoc::Create();
    Xchg_ComponentPtr pk_comp = pk_doc->CreateComponent(
        L"pk_bodies", L"pk_bodies", Xchg_Component::ComponentInternal);
    pk_doc->SetRootComponent(pk_comp);

    int node_count = 0;
    for (Xchg_Size_t i = 0; i < num_nodes; ++i) {
        Xchg_NodePtr node = root->GetNodeByIndex(i);
        if (!node || node->GetNodeType() != Xchg_Node::BodyType)
            continue;

        printf("[Info] Processing Body node at index %zu.\n", i);

        // Xchg_Body -> PK_BODY
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
        printf("[Info] Xchg->PK succeeded, PK_BODY tag: %d\n", pk_body_tag);

        // 在新 MainDoc 中创建对应 node，存储该 PK_BODY
        wchar_t node_name[64];
        swprintf(node_name, 64, L"body_%d", node_count++);
        Xchg_BodyPtr empty_body = Xchg_Body::Create();
        Xchg_NodePtr pk_node = pk_comp->CreateBodyNode(node_name, empty_body, 0);
        pk_node->SetParasolidBody(pk_body_tag);
    }

    printf("[Info] Collected %d PK_BODY nodes.\n", node_count);

    // Step 2: 遍历新 MainDoc，对每个 PK_BODY 执行 PK->X->PK 往返转换
    std::vector<PK_BODY_t> roundtrip_bodies;

    Xchg_Size_t pk_num_nodes = pk_comp->GetNumNodes();
    for (Xchg_Size_t i = 0; i < pk_num_nodes; ++i) {
        Xchg_NodePtr pk_node = pk_comp->GetNodeByIndex(i);
        if (!pk_node || pk_node->GetNodeType() != Xchg_Node::BodyType)
            continue;

        Xchg_Int32 pk_body_tag = pk_node->GetParasolidBody();
        printf("[Info] [%zu] PK->X: converting PK_BODY tag %d\n", i, pk_body_tag);

        // PK_BODY -> Xchg_Body
        Xchg_Int32 pk_to_x_err = pk_node->ConvertPKBodyToXBody();
        if (pk_to_x_err != 0) {
            fprintf(stderr, "[Error] ConvertPKBodyToXBody failed: %d\n", pk_to_x_err);
            continue;
        }
        Xchg_BodyPtr xchg_body = pk_node->GetBodyPtr();
        if (!xchg_body) {
            fprintf(stderr, "[Error] GetBodyPtr returned null after ConvertPKBodyToXBody.\n");
            continue;
        }
        printf("[Info] [%zu] PK->X succeeded.\n", i);

        // Xchg_Body -> PK_BODY
        Xchg_Int32 x_to_pk_err = pk_node->ConvertToPKBody();
        if (x_to_pk_err != 0) {
            fprintf(stderr, "[Error] X->PK ConvertToPKBody failed: %d\n", x_to_pk_err);
            continue;
        }
        Xchg_Int32 rt_pk_body = pk_node->GetParasolidBody();
        if (rt_pk_body == 0) {
            fprintf(stderr, "[Error] X->PK GetParasolidBody returned null.\n");
            continue;
        }
        printf("[Info] [%zu] X->PK succeeded, new PK_BODY tag: %d\n", i, rt_pk_body);

        roundtrip_bodies.push_back(static_cast<PK_BODY_t>(rt_pk_body));
    }

    int body_count = roundtrip_bodies.size();

    if (body_count == 0) {
        fprintf(stderr, "[Warn] No bodies survived the roundtrip.\n");
    } else {
        printf("[Info] Total %d bodies after PK->X->PK roundtrip.\n", body_count);

        // 导出往返结果到 xt 文件
        PK_PART_transmit_o_t transmit_opts;
        PK_PART_transmit_o_m(transmit_opts);
        transmit_opts.transmit_format = PK_transmit_format_text_c;

        PK_ERROR_code_t err = PK_PART_transmit(body_count, roundtrip_bodies.data(), xt_path.c_str(), &transmit_opts);
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
