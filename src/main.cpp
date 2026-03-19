#include "step/translator_step.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_maindoc.hpp"
#include "xchg_component.hpp"
#include "xchg_node.hpp"
#include "util/xchg_kernel_utils.hpp"
#include "topology/xchg_body.hpp"

#include <parasolid_kernel.h>

#include "../include/export_step.hpp"
#include "../include/xchg_to_step_writer.hpp"
#include <cstdio>
#include <string>
#include <vector>

// Dump MainDoc 装配树（打印父子关系和几何信息）
// static void DumpComponent(const Xchg_ComponentPtr& comp, int depth)
// {
//     if (!comp) return;
//     std::string indent(depth * 2, ' ');
//     printf("%s[%s] nodes=%zu children=%zu\n",
//         indent.c_str(), comp->Name().c_str(),
//         comp->GetNumNodes(), comp->GetNumChildren());
//     // 打印每个 node 的 lump 和 face 数量
//     for (Xchg_Size_t ni = 0; ni < comp->GetNumNodes(); ++ni) {
//         Xchg_NodePtr node = comp->GetNodeByIndex(ni);
//         if (!node) continue;
//         Xchg_BodyPtr body = node->GetBodyPtr();
//         if (body) {
//             printf("%s  node[%zu]: lumps=%zu\n",
//                 indent.c_str(), ni, body->GetNumLumps());
//             // 统计所有 face 数量
//             Xchg_Size_t totalFaces = 0;
//             for (Xchg_Size_t li = 0; li < body->GetNumLumps(); ++li) {
//                 Xchg_LumpPtr lump = body->GetLump(li);
//                 if (lump) {
//                     for (Xchg_Size_t si = 0; si < lump->GetNumShells(); ++si) {
//                         Xchg_ShellPtr shell = lump->GetShell(si);
//                         if (shell) totalFaces += shell->GetNumFaces();
//                     }
//                 }
//             }
//             printf("%s    total faces=%zu\n", indent.c_str(), totalFaces);
//         }
//     }
//     for (Xchg_Size_t ci = 0; ci < comp->GetNumChildren(); ++ci) {
//         Xchg_ComponentInstancePtr inst = comp->GetChild(ci);
//         if (!inst || !inst->GetComponent()) continue;
//         DumpComponent(inst->GetComponent(), depth + 1);
//     }
// }

// static void DumpMainDoc(const Xchg_MainDocPtr& mainDoc)
// {
//     if (!mainDoc) return;
//     printf("\n===== MainDoc Assembly Tree =====\n");
//     DumpComponent(mainDoc->RootComponent(), 0);
//     printf("=================================\n\n");
// }

// ─────────────────────────────────────────────────────────────
// Export_xt：递归收集 PK_BODY，考虑装配变换（原地变换到世界坐标）
// 参考 amxt_stp/tests/debug/wzj.cpp 的实现逻辑
// ─────────────────────────────────────────────────────────────
static int g_xt_count = 0;
static std::vector<PK_BODY_t> g_xt_bodies;

static PK_ERROR_code_t SerializeNode_xt(
    const Xchg_ComponentPtr& comp,
    PK_TRANSF_t transfer)  // -1 表示无变换
{
    // 处理当前 component 的所有 body node
    for (Xchg_Size_t i = 0; i < comp->GetNumNodes(); ++i) {
        auto nodePtr = comp->GetNodeByIndex(i);
        if (!nodePtr) continue;

        PK_ERROR_code_t errCode = static_cast<PK_ERROR_code_t>(nodePtr->ConvertToPKBody());
        if (errCode != PK_ERROR_no_errors) {
            fprintf(stderr, "[Warn] Export_xt: ConvertToPKBody failed: %d\n", errCode);
            continue;
        }

        PK_BODY_t pk_body = static_cast<PK_BODY_t>(nodePtr->GetParasolidBody());
        if (!pk_body) continue;

        // 应用装配变换（原地变换到世界坐标）
        if (transfer != -1) {
            PK_BODY_transform_o_t opts;
            PK_BODY_transform_o_m(opts);
            PK_TOPOL_track_r_t trsftracking{};
            PK_TOPOL_local_r_t trsfresults{};
            if (auto err = PK_BODY_transform_2(pk_body, transfer, 1e-10,
                    &opts, &trsftracking, &trsfresults);
                err != PK_ERROR_no_errors) {
                fprintf(stderr, "[Warn] Export_xt: PK_BODY_transform_2 failed: %d\n", err);
            }
        }

        // 去重后加入列表
        if (std::find(g_xt_bodies.begin(), g_xt_bodies.end(), pk_body) == g_xt_bodies.end())
            g_xt_bodies.push_back(pk_body);
        ++g_xt_count;
    }

    // 递归处理子 component
    for (Xchg_Size_t i = 0; i < comp->GetNumChildren(); ++i) {
        Xchg_ComponentInstancePtr inst = comp->GetChild(i);
        if (!inst || !inst->GetComponent()) continue;
        PK_TRANSF_t trsf = 0;
        inst->GetKernelTransf(trsf);
        SerializeNode_xt(inst->GetComponent(), trsf);
    }

    return PK_ERROR_no_errors;
}

// 将 MainDoc（从 STEP 读入）转换为 PK_BODY 并导出 .x_t 文件
// 输出到 xt/ 目录，文件名为 <stem>_export
void Export_xt(Xchg_MainDocPtr* mainDoc, const std::string& input_step_path)
{
    if (!mainDoc || !*mainDoc) return;

    // 从输入路径提取文件名 stem
    auto pos_slash  = input_step_path.rfind('/');
    auto pos_bslash = input_step_path.rfind('\\');
    auto name_start = std::string::npos;
    if (pos_slash != std::string::npos && pos_bslash != std::string::npos)
        name_start = std::max(pos_slash, pos_bslash);
    else if (pos_slash  != std::string::npos) name_start = pos_slash;
    else                                       name_start = pos_bslash;
    std::string filename = (name_start != std::string::npos)
                           ? input_step_path.substr(name_start + 1) : input_step_path;
    auto ext_pos = filename.rfind('.');
    std::string stem = (ext_pos != std::string::npos) ? filename.substr(0, ext_pos) : filename;

    // 构建输出目录（exe所在目录的上三级/xt/）
    // 使用当前工作目录下的 xt/ 子目录
    std::string output_dir = "..\\..\\..\\xt\\"; // 相对 exe 路径
    std::string output_path = output_dir + stem + "_export";

    printf("[Info] Exporting MainDoc to XT: %s.x_t\n", output_path.c_str());

    const Xchg_ComponentPtr& root = (*mainDoc)->RootComponent();
    if (!root) {
        fprintf(stderr, "[Error] Export_xt: no root component\n");
        return;
    }

    // 重置全局状态
    g_xt_count = 0;
    g_xt_bodies.clear();

    // 递归收集并变换所有 PK_BODY
    SerializeNode_xt(root, -1);

    printf("[Info] Export_xt: nodes=%d  unique bodies=%zu\n",
           g_xt_count, g_xt_bodies.size());

    if (g_xt_bodies.empty()) {
        fprintf(stderr, "[Warn] Export_xt: no bodies to export\n");
        return;
    }

    // PK_PART_t 数组（PK_BODY_t 和 PK_PART_t 都是 int，但类型要匹配）
    std::vector<PK_PART_t> parts(g_xt_bodies.begin(), g_xt_bodies.end());

    PK_PART_transmit_o_t opts;
    PK_PART_transmit_o_m(opts);
    opts.transmit_format = PK_transmit_format_text_c;

    PK_ERROR_code_t err = PK_PART_transmit(
        static_cast<int>(parts.size()), parts.data(),
        output_path.c_str(), &opts);
    if (err != PK_ERROR_no_errors)
        fprintf(stderr, "[Error] Export_xt: PK_PART_transmit failed: %d\n", err);
    else
        printf("[Info] Export_xt: written to %s.x_t\n", output_path.c_str());
}

// 使用我们自己的 XchgToSTEPWriter 导出 MainDoc（支持完整装配体结构）
void Export_step(Xchg_MainDocPtr* mainDoc, const std::string& input_step_path)
{
    // 从输入路径提取文件名（不含扩展名）
    // 取 '/' 和 '\\' 中最后出现的那个作为目录分隔符
    auto pos_slash = input_step_path.rfind('/');
    auto pos_bslash = input_step_path.rfind('\\');
    auto name_start = std::string::npos;
    if (pos_slash != std::string::npos && pos_bslash != std::string::npos)
        name_start = std::max(pos_slash, pos_bslash);
    else if (pos_slash != std::string::npos)
        name_start = pos_slash;
    else
        name_start = pos_bslash;
    std::string filename = (name_start != std::string::npos)
                          ? input_step_path.substr(name_start + 1) : input_step_path;

    // 去掉扩展名
    auto ext_pos = filename.rfind('.');
    std::string stem = (ext_pos != std::string::npos) ? filename.substr(0, ext_pos) : filename;

    // 生成输出文件名：原文件名 + _export.step
    std::string output_path =  stem + "_export.step";

    printf("[Info] Exporting MainDoc to: %s (via XchgToSTEPWriter)\n", output_path.c_str());

    //DumpMainDoc(*mainDoc);

    XchgToSTEPWriter writer(output_path);
    writer.SetPrecision(1e-6);

    bool ok = writer.WriteMainDoc(*mainDoc);
    writer.Done();

    if (!ok) {
        fprintf(stderr, "[Error] XchgToSTEPWriter::WriteMainDoc failed\n");
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
        step_path = base + "resource/12148.step";
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

    size_t err_instance = 0;
    const char* err_msg = nullptr;
    AMXT_STP_ERROR_ask_message(ctx, &err_instance, &err_msg);
    if(err_msg && strlen(err_msg)>0)
    {
        fprintf(stderr, "[Error] AMXT_STP_read failed\n");
       // return -1;
    }

    AMXT_STP_ERROR_ask_message(ctx, &err_instance, &err_msg);
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

    // Export the MainDoc to XT file (STEP -> Xchg -> PK_BODY -> XT)
    Export_xt(&main_doc, step_path);




#if 0 //pk转x流程

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

#endif
    return 0;
}
