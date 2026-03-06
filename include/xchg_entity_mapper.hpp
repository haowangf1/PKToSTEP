#pragma once

#include <unordered_map>
#include <memory>

// Forward declarations
template<typename T>
class SmartPtr;
class Xchg_Vertex;
class Xchg_Edge;
class Xchg_Surface;
class Xchg_Curve;
class Xchg_Point;

typedef SmartPtr<Xchg_Vertex> Xchg_VertexPtr;
typedef SmartPtr<Xchg_Edge> Xchg_EdgePtr;
typedef SmartPtr<Xchg_Surface> Xchg_SurfacePtr;
typedef SmartPtr<Xchg_Curve> Xchg_CurvePtr;
typedef SmartPtr<Xchg_Point> Xchg_PointPtr;



/**
 * @brief Xchg 实体到 STEP 实体编号的映射器
 * 用于去重和复用 Vertex/Edge/Surface/Curve/Point
 */
class XchgEntityMapper {
public:
    XchgEntityMapper();
    ~XchgEntityMapper() = default;

    // 获取或分配实体编号
    int GetOrAllocate(const Xchg_VertexPtr& vertex);
    int GetOrAllocate(const Xchg_EdgePtr& edge);
    int GetOrAllocate(const Xchg_SurfacePtr& surface);
    int GetOrAllocate(const Xchg_CurvePtr& curve);
    int GetOrAllocate(const Xchg_PointPtr& point);

    // 检查是否已映射
    bool HasMapping(const Xchg_VertexPtr& vertex) const;
    bool HasMapping(const Xchg_EdgePtr& edge) const;
    bool HasMapping(const Xchg_SurfacePtr& surface) const;
    bool HasMapping(const Xchg_CurvePtr& curve) const;
    bool HasMapping(const Xchg_PointPtr& point) const;

    // 获取已映射的编号（不分配新编号）
    int GetMapping(const Xchg_VertexPtr& vertex) const;
    int GetMapping(const Xchg_EdgePtr& edge) const;
    int GetMapping(const Xchg_SurfacePtr& surface) const;
    int GetMapping(const Xchg_CurvePtr& curve) const;
    int GetMapping(const Xchg_PointPtr& point) const;

    // 分配新编号
    int AllocateNewId();

    // 获取当前编号
    int GetCurrentId() const { return m_nextId; }

    // 清空映射
    void Clear();

private:
    std::unordered_map<void*, int> m_vertexMap;
    std::unordered_map<void*, int> m_edgeMap;
    std::unordered_map<void*, int> m_surfaceMap;
    std::unordered_map<void*, int> m_curveMap;
    std::unordered_map<void*, int> m_pointMap;

    int m_nextId;
};

