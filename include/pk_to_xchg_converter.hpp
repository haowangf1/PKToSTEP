#ifndef PK_TO_XCHG_CONVERTER_HPP
#define PK_TO_XCHG_CONVERTER_HPP

#include <unordered_map>
#include <string>
#include <functional>

#include "parasolid_kernel.h"
#include "xchg_error_code.hpp"

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

    STEPExport_ErrorCode Convert(PK_BODY_t pk_body, Xchg_BodyPtr* body);

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

    // PK error -> STEPExport_ErrorCode, returns STEP_OK on no error
    STEPExport_ErrorCode PKErr(PK_ERROR_code_t err, const char* context);

    // topology conversion
    STEPExport_ErrorCode ConvertRegions(PK_BODY_t pk_body, PK_BODY_type_t body_type);
    STEPExport_ErrorCode ConvertShell(PK_SHELL_t pk_shell, Xchg_ShellPtr* shell);
    STEPExport_ErrorCode ConvertFace(PK_FACE_t pk_face, Xchg_FacePtr* face);
    STEPExport_ErrorCode ConvertLoop(PK_LOOP_t pk_loop, Xchg_LoopPtr* loop);
    STEPExport_ErrorCode ConvertFin(PK_FIN_t pk_fin, Xchg_CoedgePtr* coedge);
    STEPExport_ErrorCode ConvertEdge(PK_EDGE_t pk_edge, Xchg_EdgePtr* edge);
    STEPExport_ErrorCode ConvertVertex(PK_VERTEX_t pk_vertex, Xchg_VertexPtr* vertex);

    // geometry conversion
    STEPExport_ErrorCode ConvertSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve);
    STEPExport_ErrorCode ConvertPoint(PK_POINT_t pk_point, Xchg_PointPtr* point);

    // surface type dispatch
    STEPExport_ErrorCode ConvertPlaneSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertCylindricalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertConicalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertSphericalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertToroidalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertNurbsSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertSweptSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertSpunSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);
    STEPExport_ErrorCode ConvertOffsetSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface);

    // curve type dispatch
    STEPExport_ErrorCode ConvertLineCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve);
    STEPExport_ErrorCode ConvertCircleCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve);
    STEPExport_ErrorCode ConvertEllipseCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve);
    STEPExport_ErrorCode ConvertNurbsCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve);

    // shell type helper
    void AddShellToLump(const Xchg_ShellPtr& shell, const Xchg_LumpPtr& lump,
                        bool is_solid_region, bool is_infinite_void, PK_BODY_type_t body_type,
                        PK_SHELL_type_t shell_type);

    // logging
    void Log(const std::string& msg);

    // save entitys in pk that has aleady been translated
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
