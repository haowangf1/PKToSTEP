#include "test_utils.hpp"
#include "step/translator_step.hpp"
#include "xchg_maindoc.hpp"
#include "xchg_component.hpp"
#include "xchg_node.hpp"
#include "topology/xchg_lump.hpp"
#include "topology/xchg_shell.hpp"
#include "topology/xchg_face.hpp"
#include "topology/xchg_loop.hpp"
#include "topology/xchg_coedge.hpp"
#include "topology/xchg_edge.hpp"
#include "topology/xchg_vertex.hpp"
#include <set>
#include <cmath>
#include <algorithm>

namespace TestUtils {

// ============================================================================
// 内存管理辅助
// ============================================================================

struct PKMemGuard {
    void* ptr = nullptr;
    ~PKMemGuard() { if (ptr) PK_MEMORY_free(ptr); }
};

// ============================================================================
// 文件加载
// ============================================================================

PK_BODY_t LoadStepFile(const std::string& filename) {
    // 创建 STEP 上下文
    AMXT_STP_CTX_context_p ctx = nullptr;
    AMXT_STP_ERROR_code_t stp_err = AMXT_STP_CTX_create(&ctx);
    if (stp_err != AMXT_STP_ERROR_no_errors) {
        return PK_ENTITY_null;
    }

    // 创建读取选项
    AMXT_STP_read_o_t read_opts;
    AMXT_STP_read_o_m(read_opts);

    // 创建 MainDoc
    Xchg_MainDocPtr main_doc = Xchg_MainDoc::Create();

    // 读取 STEP 文件
    std::string filepath = "../resource/" + filename;
    stp_err = AMXT_STP_read(ctx, filepath.c_str(), &read_opts, &main_doc);
    if (stp_err != AMXT_STP_ERROR_no_errors) {
        AMXT_STP_CTX_destory(ctx);
        return PK_ENTITY_null;
    }

    // 获取根组件
    const Xchg_ComponentPtr& root = main_doc->RootComponent();
    if (!root) {
        AMXT_STP_CTX_destory(ctx);
        return PK_ENTITY_null;
    }

    // 遍历节点找到第一个 Body
    Xchg_Size_t num_nodes = root->GetNumNodes();
    for (Xchg_Size_t i = 0; i < num_nodes; ++i) {
        Xchg_NodePtr node;
        root->GetNode(i, node);
        if (!node) continue;

        Xchg_BodyPtr xchg_body = node->GetBody();
        if (xchg_body) {
            // 转换为 PK_BODY
            PK_BODY_t pk_body = xchg_body->ConvertToPKBody();
            AMXT_STP_CTX_destory(ctx);
            return pk_body;
        }
    }

    AMXT_STP_CTX_destory(ctx);
    return PK_ENTITY_null;
}

// ============================================================================
// 拓扑统计 - PK_BODY
// ============================================================================

int CountFaces(PK_BODY_t body) {
    if (body == PK_ENTITY_null) return 0;

    int n_faces = 0;
    PK_FACE_t* faces = nullptr;
    PKMemGuard guard;

    PK_ERROR_code_t err = PK_BODY_ask_faces(body, &n_faces, &faces);
    if (err != PK_ERROR_no_errors) return 0;

    guard.ptr = faces;
    return n_faces;
}

int CountEdges(PK_BODY_t body) {
    if (body == PK_ENTITY_null) return 0;

    int n_edges = 0;
    PK_EDGE_t* edges = nullptr;
    PKMemGuard guard;

    PK_ERROR_code_t err = PK_BODY_ask_edges(body, &n_edges, &edges);
    if (err != PK_ERROR_no_errors) return 0;

    guard.ptr = edges;
    return n_edges;
}

int CountVertices(PK_BODY_t body) {
    if (body == PK_ENTITY_null) return 0;

    int n_vertices = 0;
    PK_VERTEX_t* vertices = nullptr;
    PKMemGuard guard;

    PK_ERROR_code_t err = PK_BODY_ask_vertices(body, &n_vertices, &vertices);
    if (err != PK_ERROR_no_errors) return 0;

    guard.ptr = vertices;
    return n_vertices;
}

// ============================================================================
// 拓扑统计 - Xchg_Body
// ============================================================================

int CountXchgFaces(Xchg_BodyPtr body) {
    if (!body) return 0;

    int count = 0;
    Xchg_Size_t nLumps = body->GetNumLumps();
    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        if (!lump) continue;

        Xchg_Size_t nShells = lump->GetNumShells();
        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);
            if (!shell) continue;

            count += shell->GetNumFaces(DtkFaceType_Bounded);
        }
    }
    return count;
}

int CountUniqueXchgEdges(Xchg_BodyPtr body) {
    if (!body) return 0;

    std::set<Xchg_EdgePtr> unique_edges;
    Xchg_Size_t nLumps = body->GetNumLumps();
    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        if (!lump) continue;

        Xchg_Size_t nShells = lump->GetNumShells();
        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);
            if (!shell) continue;

            Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
            for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                Xchg_FacePtr face;
                Xchg_bool faceOrient;
                shell->GetFace(fi, face, faceOrient);
                if (!face) continue;

                Xchg_Size_t nLoops = face->GetNumLoops();
                for (Xchg_Size_t lli = 0; lli < nLoops; ++lli) {
                    Xchg_LoopPtr loop;
                    face->GetLoop(lli, loop);
                    if (!loop) continue;

                    Xchg_Size_t nCoedges = loop->GetNumCoedges();
                    for (Xchg_Size_t ci = 0; ci < nCoedges; ++ci) {
                        Xchg_CoedgePtr coedge;
                        Xchg_bool coedgeOrient;
                        loop->GetCoedge(ci, coedge, coedgeOrient);
                        if (!coedge) continue;

                        Xchg_EdgePtr edge;
                        coedge->GetEdge(edge);
                        if (edge) unique_edges.insert(edge);
                    }
                }
            }
        }
    }
    return unique_edges.size();
}

int CountUniqueXchgVertices(Xchg_BodyPtr body) {
    if (!body) return 0;

    std::set<Xchg_VertexPtr> unique_vertices;
    Xchg_Size_t nLumps = body->GetNumLumps();
    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        if (!lump) continue;

        Xchg_Size_t nShells = lump->GetNumShells();
        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);
            if (!shell) continue;

            Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
            for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                Xchg_FacePtr face;
                Xchg_bool faceOrient;
                shell->GetFace(fi, face, faceOrient);
                if (!face) continue;

                Xchg_Size_t nLoops = face->GetNumLoops();
                for (Xchg_Size_t lli = 0; lli < nLoops; ++lli) {
                    Xchg_LoopPtr loop;
                    face->GetLoop(lli, loop);
                    if (!loop) continue;

                    Xchg_Size_t nCoedges = loop->GetNumCoedges();
                    for (Xchg_Size_t ci = 0; ci < nCoedges; ++ci) {
                        Xchg_CoedgePtr coedge;
                        Xchg_bool coedgeOrient;
                        loop->GetCoedge(ci, coedge, coedgeOrient);
                        if (!coedge) continue;

                        Xchg_EdgePtr edge;
                        coedge->GetEdge(edge);
                        if (edge) {
                            Xchg_VertexPtr start, end;
                            edge->GetStartVertex(start);
                            edge->GetEndVertex(end);
                            if (start) unique_vertices.insert(start);
                            if (end) unique_vertices.insert(end);
                        }
                    }
                }
            }
        }
    }
    return unique_vertices.size();
}

// ============================================================================
// 拓扑比较
// ============================================================================

bool CompareTopology(PK_BODY_t body1, PK_BODY_t body2) {
    if (body1 == PK_ENTITY_null || body2 == PK_ENTITY_null) {
        return false;
    }

    int faces1 = CountFaces(body1);
    int faces2 = CountFaces(body2);
    if (faces1 != faces2) return false;

    int edges1 = CountEdges(body1);
    int edges2 = CountEdges(body2);
    if (edges1 != edges2) return false;

    int vertices1 = CountVertices(body1);
    int vertices2 = CountVertices(body2);
    if (vertices1 != vertices2) return false;

    return true;
}

// ============================================================================
// 几何提取
// ============================================================================

std::vector<PK_VECTOR_t> ExtractVertices(PK_BODY_t body) {
    std::vector<PK_VECTOR_t> result;
    if (body == PK_ENTITY_null) return result;

    int n_vertices = 0;
    PK_VERTEX_t* vertices = nullptr;
    PKMemGuard guard;

    PK_ERROR_code_t err = PK_BODY_ask_vertices(body, &n_vertices, &vertices);
    if (err != PK_ERROR_no_errors) return result;

    guard.ptr = vertices;

    for (int i = 0; i < n_vertices; ++i) {
        PK_POINT_t pk_point = PK_ENTITY_null;
        err = PK_VERTEX_ask_point(vertices[i], &pk_point);
        if (err == PK_ERROR_no_errors && pk_point != PK_ENTITY_null) {
            PK_POINT_sf_t point_sf;
            err = PK_POINT_ask(pk_point, &point_sf);
            if (err == PK_ERROR_no_errors) {
                result.push_back(point_sf.position);
            }
        }
    }

    return result;
}

BoundingBox ComputeBoundingBox(PK_BODY_t body) {
    BoundingBox bbox;
    bbox.min_x = bbox.min_y = bbox.min_z = 1e30;
    bbox.max_x = bbox.max_y = bbox.max_z = -1e30;

    auto vertices = ExtractVertices(body);
    for (const auto& v : vertices) {
        bbox.min_x = std::min(bbox.min_x, v.coord[0]);
        bbox.min_y = std::min(bbox.min_y, v.coord[1]);
        bbox.min_z = std::min(bbox.min_z, v.coord[2]);
        bbox.max_x = std::max(bbox.max_x, v.coord[0]);
        bbox.max_y = std::max(bbox.max_y, v.coord[1]);
        bbox.max_z = std::max(bbox.max_z, v.coord[2]);
    }

    return bbox;
}

// ============================================================================
// 几何比较
// ============================================================================

bool VectorsEqual(const PK_VECTOR_t& v1, const PK_VECTOR_t& v2, double tolerance) {
    return std::abs(v1.coord[0] - v2.coord[0]) < tolerance &&
           std::abs(v1.coord[1] - v2.coord[1]) < tolerance &&
           std::abs(v1.coord[2] - v2.coord[2]) < tolerance;
}

bool CompareGeometry(PK_BODY_t body1, PK_BODY_t body2, double tolerance) {
    auto vertices1 = ExtractVertices(body1);
    auto vertices2 = ExtractVertices(body2);

    if (vertices1.size() != vertices2.size()) {
        return false;
    }

    // 简单比较：假设顶点顺序一致
    // 更严格的比较需要考虑顶点可能的重新排序
    for (size_t i = 0; i < vertices1.size(); ++i) {
        if (!VectorsEqual(vertices1[i], vertices2[i], tolerance)) {
            return false;
        }
    }

    return true;
}

// ============================================================================
// Parasolid 会话管理
// ============================================================================

bool InitializePK() {
    PK_SESSION_start_o_t start_opts;
    PK_SESSION_start_o_m(start_opts);

    PK_ERROR_code_t err = PK_SESSION_start(&start_opts);
    return (err == PK_ERROR_no_errors);
}

void TerminatePK() {
    PK_SESSION_stop();
}

} // namespace TestUtils
