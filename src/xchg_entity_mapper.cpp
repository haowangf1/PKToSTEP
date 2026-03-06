#include "../include/xchg_entity_mapper.hpp"
#include "topology/xchg_vertex.hpp"
#include "topology/xchg_edge.hpp"
#include "geom/surface/xchg_surface.hpp"
#include "geom/curve/xchg_curve.hpp"
#include "geom/xchg_point.hpp"



XchgEntityMapper::XchgEntityMapper() : m_nextId(1) {
}

int XchgEntityMapper::AllocateNewId() {
    return m_nextId++;
}

int XchgEntityMapper::GetOrAllocate(const Xchg_VertexPtr& vertex) {
    if (!vertex) return 0;

    void* key = vertex.get();
    auto it = m_vertexMap.find(key);
    if (it != m_vertexMap.end()) {
        return it->second;
    }

    int id = AllocateNewId();
    m_vertexMap[key] = id;
    return id;
}

int XchgEntityMapper::GetOrAllocate(const Xchg_EdgePtr& edge) {
    if (!edge) return 0;

    void* key = edge.get();
    auto it = m_edgeMap.find(key);
    if (it != m_edgeMap.end()) {
        return it->second;
    }

    int id = AllocateNewId();
    m_edgeMap[key] = id;
    return id;
}

int XchgEntityMapper::GetOrAllocate(const Xchg_SurfacePtr& surface) {
    if (!surface) return 0;

    void* key = surface.get();
    auto it = m_surfaceMap.find(key);
    if (it != m_surfaceMap.end()) {
        return it->second;
    }

    int id = AllocateNewId();
    m_surfaceMap[key] = id;
    return id;
}

int XchgEntityMapper::GetOrAllocate(const Xchg_CurvePtr& curve) {
    if (!curve) return 0;

    void* key = curve.get();
    auto it = m_curveMap.find(key);
    if (it != m_curveMap.end()) {
        return it->second;
    }

    int id = AllocateNewId();
    m_curveMap[key] = id;
    return id;
}

int XchgEntityMapper::GetOrAllocate(const Xchg_PointPtr& point) {
    if (!point) return 0;

    void* key = point.get();
    auto it = m_pointMap.find(key);
    if (it != m_pointMap.end()) {
        return it->second;
    }

    int id = AllocateNewId();
    m_pointMap[key] = id;
    return id;
}

bool XchgEntityMapper::HasMapping(const Xchg_VertexPtr& vertex) const {
    if (!vertex) return false;
    return m_vertexMap.find(vertex.get()) != m_vertexMap.end();
}

bool XchgEntityMapper::HasMapping(const Xchg_EdgePtr& edge) const {
    if (!edge) return false;
    return m_edgeMap.find(edge.get()) != m_edgeMap.end();
}

bool XchgEntityMapper::HasMapping(const Xchg_SurfacePtr& surface) const {
    if (!surface) return false;
    return m_surfaceMap.find(surface.get()) != m_surfaceMap.end();
}

bool XchgEntityMapper::HasMapping(const Xchg_CurvePtr& curve) const {
    if (!curve) return false;
    return m_curveMap.find(curve.get()) != m_curveMap.end();
}

bool XchgEntityMapper::HasMapping(const Xchg_PointPtr& point) const {
    if (!point) return false;
    return m_pointMap.find(point.get()) != m_pointMap.end();
}

int XchgEntityMapper::GetMapping(const Xchg_VertexPtr& vertex) const {
    if (!vertex) return 0;
    auto it = m_vertexMap.find(vertex.get());
    return (it != m_vertexMap.end()) ? it->second : 0;
}

int XchgEntityMapper::GetMapping(const Xchg_EdgePtr& edge) const {
    if (!edge) return 0;
    auto it = m_edgeMap.find(edge.get());
    return (it != m_edgeMap.end()) ? it->second : 0;
}

int XchgEntityMapper::GetMapping(const Xchg_SurfacePtr& surface) const {
    if (!surface) return 0;
    auto it = m_surfaceMap.find(surface.get());
    return (it != m_surfaceMap.end()) ? it->second : 0;
}

int XchgEntityMapper::GetMapping(const Xchg_CurvePtr& curve) const {
    if (!curve) return 0;
    auto it = m_curveMap.find(curve.get());
    return (it != m_curveMap.end()) ? it->second : 0;
}

int XchgEntityMapper::GetMapping(const Xchg_PointPtr& point) const {
    if (!point) return 0;
    auto it = m_pointMap.find(point.get());
    return (it != m_pointMap.end()) ? it->second : 0;
}

void XchgEntityMapper::Clear() {
    m_vertexMap.clear();
    m_edgeMap.clear();
    m_surfaceMap.clear();
    m_curveMap.clear();
    m_pointMap.clear();
    m_nextId = 1;
}


