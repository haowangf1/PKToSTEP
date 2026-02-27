#ifndef PK_TO_XCHG_CONVERTER_HPP
#define PK_TO_XCHG_CONVERTER_HPP

#include <unordered_map>
#include <string>
#include <functional>

#include "parasolid_kernel.h"

#include "topology/xchg_body.hpp"
#include "topology/xchg_lump.hpp"
#include "topology/xchg_shell.hpp"
#include "topology/xchg_face.hpp"
#include "topology/xchg_loop.hpp"
#include "topology/xchg_coedge.hpp"
#include "topology/xchg_edge.hpp"
#include "topology/xchg_vertex.hpp"
#include "geom/geom.hpp"

class PKToXchgConverter
{
public:
    PKToXchgConverter() = default;
    ~PKToXchgConverter() = default;

    PKToXchgConverter(const PKToXchgConverter&) = delete;
    PKToXchgConverter& operator=(const PKToXchgConverter&) = delete;

    Xchg_BodyPtr Convert(PK_BODY_t pk_body);

    using LogCallback = std::function<void(const std::string&)>;
    void SetLogCallback(LogCallback cb) { log_callback_ = std::move(cb); }

private:
    // PK memory RAII guard
    struct PKMemGuard {
        void* ptr = nullptr;
        ~PKMemGuard() { if (ptr) PK_MEMORY_free(ptr); }
        PKMemGuard() = default;
        explicit PKMemGuard(void* p) : ptr(p) {}
        PKMemGuard(const PKMemGuard&) = delete;
        PKMemGuard& operator=(const PKMemGuard&) = delete;
    };

    // topology conversion
    void ConvertRegions(PK_BODY_t pk_body, PK_BODY_type_t body_type);
    Xchg_ShellPtr ConvertShell(PK_SHELL_t pk_shell);
    Xchg_FacePtr ConvertFace(PK_FACE_t pk_face);
    Xchg_LoopPtr ConvertLoop(PK_LOOP_t pk_loop);
    Xchg_CoedgePtr ConvertFin(PK_FIN_t pk_fin);
    Xchg_EdgePtr ConvertEdge(PK_EDGE_t pk_edge);
    Xchg_VertexPtr ConvertVertex(PK_VERTEX_t pk_vertex);

    // geometry conversion
    Xchg_SurfacePtr ConvertSurface(PK_SURF_t pk_surf);
    Xchg_CurvePtr ConvertCurve(PK_CURVE_t pk_curve);
    Xchg_PointPtr ConvertPoint(PK_POINT_t pk_point);

    // surface type dispatch
    Xchg_SurfacePtr ConvertPlaneSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertCylindricalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertConicalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertSphericalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertToroidalSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertNurbsSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertSweptSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertSpunSurface(PK_SURF_t pk_surf);
    Xchg_SurfacePtr ConvertOffsetSurface(PK_SURF_t pk_surf);

    // curve type dispatch
    Xchg_CurvePtr ConvertLineCurve(PK_CURVE_t pk_curve);
    Xchg_CurvePtr ConvertCircleCurve(PK_CURVE_t pk_curve);
    Xchg_CurvePtr ConvertEllipseCurve(PK_CURVE_t pk_curve);
    Xchg_CurvePtr ConvertNurbsCurve(PK_CURVE_t pk_curve);

    // shell type helper
    void AddShellToLump(const Xchg_ShellPtr& shell, const Xchg_LumpPtr& lump,
                        bool is_solid_region, bool is_infinite_void, PK_BODY_type_t body_type,
                        PK_SHELL_type_t shell_type);

    // logging
    void Log(const std::string& msg);

    // error check helper — returns true on error
    bool PKCheck(PK_ERROR_code_t err, const char* context);

    // entity dedup maps
    std::unordered_map<PK_ENTITY_t, Xchg_FacePtr>    pk_face_map_;
    std::unordered_map<PK_ENTITY_t, Xchg_EdgePtr>    pk_edge_map_;
    std::unordered_map<PK_ENTITY_t, Xchg_VertexPtr>  pk_vertex_map_;
    std::unordered_map<PK_ENTITY_t, Xchg_SurfacePtr> pk_surface_map_;
    std::unordered_map<PK_ENTITY_t, Xchg_CurvePtr>   pk_curve_map_;

    Xchg_BodyPtr current_body_;
    LogCallback log_callback_;

    void ClearMaps();
};

#endif // PK_TO_XCHG_CONVERTER_HPP
