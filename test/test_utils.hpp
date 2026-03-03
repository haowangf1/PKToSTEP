#pragma once

#include <string>
#include <vector>
#include "parasolid_kernel.h"
#include "topology/xchg_body.hpp"

namespace TestUtils {

// ============================================================================
// 文件加载
// ============================================================================

/**
 * @brief 从 STEP 文件加载 PK_BODY
 * @param filename STEP 文件路径（相对于 resource/ 目录）
 * @return PK_BODY_t 加载的 body，失败返回 PK_ENTITY_null
 */
PK_BODY_t LoadStepFile(const std::string& filename);

// ============================================================================
// 拓扑统计
// ============================================================================

/**
 * @brief 统计 PK_BODY 中的 Face 数量
 */
int CountFaces(PK_BODY_t body);

/**
 * @brief 统计 PK_BODY 中的 Edge 数量
 */
int CountEdges(PK_BODY_t body);

/**
 * @brief 统计 PK_BODY 中的 Vertex 数量
 */
int CountVertices(PK_BODY_t body);

/**
 * @brief 统计 Xchg_Body 中的 Face 数量
 */
int CountXchgFaces(Xchg_BodyPtr body);

/**
 * @brief 统计 Xchg_Body 中的唯一 Edge 数量
 */
int CountUniqueXchgEdges(Xchg_BodyPtr body);

/**
 * @brief 统计 Xchg_Body 中的唯一 Vertex 数量
 */
int CountUniqueXchgVertices(Xchg_BodyPtr body);

// ============================================================================
// 拓扑比较
// ============================================================================

/**
 * @brief 比较两个 PK_BODY 的拓扑结构是否一致
 * @param body1 第一个 body
 * @param body2 第二个 body
 * @return true 如果拓扑一致（Face/Edge/Vertex 数量相同）
 */
bool CompareTopology(PK_BODY_t body1, PK_BODY_t body2);

// ============================================================================
// 几何提取
// ============================================================================

/**
 * @brief 提取 PK_BODY 中所有 Vertex 的坐标
 * @param body PK_BODY
 * @return 顶点坐标列表
 */
std::vector<PK_VECTOR_t> ExtractVertices(PK_BODY_t body);

/**
 * @brief 包围盒结构
 */
struct BoundingBox {
    double min_x, min_y, min_z;
    double max_x, max_y, max_z;

    bool IsValid() const {
        return min_x <= max_x && min_y <= max_y && min_z <= max_z;
    }
};

/**
 * @brief 计算 PK_BODY 的包围盒
 */
BoundingBox ComputeBoundingBox(PK_BODY_t body);

// ============================================================================
// 几何比较
// ============================================================================

/**
 * @brief 比较两个 PK_BODY 的几何是否一致
 * @param body1 第一个 body
 * @param body2 第二个 body
 * @param tolerance 容差（默认 1e-6）
 * @return true 如果几何一致（顶点坐标误差在容差内）
 */
bool CompareGeometry(PK_BODY_t body1, PK_BODY_t body2, double tolerance = 1e-6);

/**
 * @brief 比较两个向量是否相等（在容差内）
 */
bool VectorsEqual(const PK_VECTOR_t& v1, const PK_VECTOR_t& v2, double tolerance = 1e-6);

// ============================================================================
// 辅助工具
// ============================================================================

/**
 * @brief 初始化 Parasolid 会话（测试前调用）
 */
bool InitializePK();

/**
 * @brief 终止 Parasolid 会话（测试后调用）
 */
void TerminatePK();

/**
 * @brief RAII 包装器，自动管理 PK 会话
 */
class PKSession {
public:
    PKSession() { InitializePK(); }
    ~PKSession() { TerminatePK(); }
};

} // namespace TestUtils
