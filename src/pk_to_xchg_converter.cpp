#include "pk_to_xchg_converter.hpp"
#include <cstdio>

// ---------------------------------------------------------------------------
// Utilities
// ---------------------------------------------------------------------------

void PKToXchgConverter::Log(const std::string& msg)
{
    if (log_callback_)
        log_callback_(msg);
}

STEPExport_ErrorCode PKToXchgConverter::PKErr(PK_ERROR_code_t err, const char* context)
{
    if (err == PK_ERROR_no_errors)
        return STEP_OK;
    char buf[256];
    snprintf(buf, sizeof(buf), "PK error %d at %s", static_cast<int>(err), context);
    Log(buf);
    return STEP_ERR_KERNEL;
}

void PKToXchgConverter::ClearMaps()
{
    pk_face_map_.clear();
    pk_edge_map_.clear();
    pk_vertex_map_.clear();
    pk_surface_map_.clear();
    pk_curve_map_.clear();
    current_body_ = nullptr;
}

// ---------------------------------------------------------------------------
// Body
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::Convert(PK_BODY_t pk_body, Xchg_BodyPtr* body)
{
    ClearMaps();
    *body = nullptr;

    PK_BODY_type_t body_type = PK_BODY_type_solid_c;
    STEPExport_ErrorCode rc = PKErr(PK_BODY_ask_type(pk_body, &body_type), "PK_BODY_ask_type");
    if (rc != STEP_OK)
        return rc;

    current_body_ = Xchg_Body::Create();

    Xchg_BodyType xchg_type = Xchg_BodyType_Solid;
    switch (body_type) {
    case PK_BODY_type_solid_c:   xchg_type = Xchg_BodyType_Solid;   break;
    case PK_BODY_type_sheet_c:   xchg_type = Xchg_BodyType_Sheet;   break;
    case PK_BODY_type_wire_c:    xchg_type = Xchg_BodyType_Wire;    break;
    case PK_BODY_type_acorn_c:
    case PK_BODY_type_minimum_c: xchg_type = Xchg_BodyType_Acorn;   break;
    case PK_BODY_type_empty_c:   xchg_type = Xchg_BodyType_Empty;   break;
    case PK_BODY_type_general_c: xchg_type = Xchg_BodyType_General; break;
    default:                     xchg_type = Xchg_BodyType_General; break;
    }
    current_body_->SetBodyType(xchg_type);

    rc = ConvertRegions(pk_body, body_type);
    if (rc != STEP_OK)
        return rc;

    *body = current_body_;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Region -> Lump (all regions, no skipping)
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertRegions(PK_BODY_t pk_body, PK_BODY_type_t body_type)
{
    int n_regions = 0;
    PK_REGION_t* regions = nullptr;
    STEPExport_ErrorCode rc = PKErr(PK_BODY_ask_regions(pk_body, &n_regions, &regions), "PK_BODY_ask_regions");
    if (rc != STEP_OK)
        return rc;
    PKMemGuard regions_guard(regions);

    for (int ri = 0; ri < n_regions; ++ri) {
        PK_REGION_t region = regions[ri];

        PK_LOGICAL_t is_solid = PK_LOGICAL_false;
        rc = PKErr(PK_REGION_is_solid(region, &is_solid), "PK_REGION_is_solid");
        if (rc != STEP_OK)
            return rc;

        bool is_infinite_void = (ri == 0 && is_solid == PK_LOGICAL_false);

        Xchg_LumpPtr lump = Xchg_Lump::Create(current_body_);
        current_body_->AddLump(lump);

        int n_shells = 0;
        PK_SHELL_t* shells = nullptr;
        rc = PKErr(PK_REGION_ask_shells(region, &n_shells, &shells), "PK_REGION_ask_shells");
        if (rc != STEP_OK)
            return rc;
        PKMemGuard shells_guard(shells);

        for (int si = 0; si < n_shells; ++si) {
            PK_SHELL_type_t shell_type = PK_SHELL_type_wireframe_free_c;
            rc = PKErr(PK_SHELL_ask_type(shells[si], &shell_type), "PK_SHELL_ask_type");
            if (rc != STEP_OK)
                return rc;

            Xchg_ShellPtr xchg_shell;
            rc = ConvertShell(shells[si], &xchg_shell);
            if (rc != STEP_OK)
                return rc;

            if (!xchg_shell)
                continue;

            AddShellToLump(xchg_shell, lump,
                           is_solid == PK_LOGICAL_true,
                           is_infinite_void,
                           body_type, shell_type);
        }
    }

    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Shell add strategy
// ---------------------------------------------------------------------------

void PKToXchgConverter::AddShellToLump(
    const Xchg_ShellPtr& shell, const Xchg_LumpPtr& lump,
    bool is_solid_region, bool is_infinite_void,
    PK_BODY_type_t body_type, PK_SHELL_type_t shell_type)
{
    if (shell_type == PK_SHELL_type_wireframe_c ||
        shell_type == PK_SHELL_type_acorn_c) {
        lump->AddWireShell(shell);
        return;
    }

    if (is_solid_region) {
        lump->AddOuterShell(shell);
        return;
    }

    // void region
    if (is_infinite_void) {
        switch (body_type) {
        case PK_BODY_type_solid_c:
            lump->AddOuterShell(shell);
            break;
        case PK_BODY_type_sheet_c:
            lump->AddOpenShell(shell);
            break;
        case PK_BODY_type_wire_c:
        case PK_BODY_type_acorn_c:
        case PK_BODY_type_minimum_c:
            lump->AddWireShell(shell);
            break;
        default:
            lump->AddOpenShell(shell);
            break;
        }
    } else {
        lump->AddInnerShell(shell);
    }
}

// ---------------------------------------------------------------------------
// Shell
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertShell(PK_SHELL_t pk_shell, Xchg_ShellPtr* shell)
{
    *shell = nullptr;

    PK_SHELL_type_t shell_type = PK_SHELL_type_wireframe_free_c;
    STEPExport_ErrorCode rc = PKErr(PK_SHELL_ask_type(pk_shell, &shell_type), "PK_SHELL_ask_type");
    if (rc != STEP_OK)
        return rc;

    Xchg_ShellPtr xchg_shell = Xchg_Shell::Create(current_body_);

    if (shell_type == PK_SHELL_type_wireframe_free_c ||
        shell_type == PK_SHELL_type_mixed_c) {
        int n_faces = 0;
        PK_FACE_t* faces = nullptr;
        PK_LOGICAL_t* orients = nullptr;
        rc = PKErr(PK_SHELL_ask_oriented_faces(pk_shell, &n_faces, &faces, &orients),
                   "PK_SHELL_ask_oriented_faces");
        if (rc != STEP_OK)
            return rc;
        PKMemGuard faces_guard(faces);
        PKMemGuard orients_guard(orients);

        for (int i = 0; i < n_faces; ++i) {
            Xchg_FacePtr xchg_face;
            rc = ConvertFace(faces[i], &xchg_face);
            if (rc != STEP_OK)
                return rc;

            if (!xchg_face)
                continue;

            PK_SURF_t surf = PK_ENTITY_null;
            PK_LOGICAL_t face_orient = PK_LOGICAL_true;
            rc = PKErr(PK_FACE_ask_oriented_surf(faces[i], &surf, &face_orient),
                       "PK_FACE_ask_oriented_surf");
            if (rc != STEP_OK)
                return rc;

            bool same = (orients[i] == face_orient);
            xchg_shell->AddFace(xchg_face, same ? XCHG_TRUE : XCHG_FALSE);
        }
    }

    if (shell_type == PK_SHELL_type_wireframe_c ||
        shell_type == PK_SHELL_type_mixed_c) {
        int n_edges = 0;
        PK_EDGE_t* edges = nullptr;
        rc = PKErr(PK_SHELL_ask_wireframe_edges(pk_shell, &n_edges, &edges),
                   "PK_SHELL_ask_wireframe_edges");
        if (rc != STEP_OK)
            return rc;
        PKMemGuard edges_guard(edges);

        Xchg_vector<Xchg_EdgePtr> xchg_edges;
        for (int i = 0; i < n_edges; ++i) {
            Xchg_EdgePtr e;
            rc = ConvertEdge(edges[i], &e);
            if (rc != STEP_OK)
                return rc;
            if (e)
                xchg_edges.push_back(e);
        }
        if (!xchg_edges.empty())
            xchg_shell->AddWire(xchg_edges);
    }

    if (shell_type == PK_SHELL_type_acorn_c) {
        PK_VERTEX_t vtx = PK_ENTITY_null;
        rc = PKErr(PK_SHELL_ask_acorn_vertex(pk_shell, &vtx), "PK_SHELL_ask_acorn_vertex");
        if (rc != STEP_OK)
            return rc;
        if (vtx != PK_ENTITY_null) {
            Xchg_VertexPtr xv;
            rc = ConvertVertex(vtx, &xv);
            if (rc != STEP_OK)
                return rc;
            if (xv) {
                Xchg_vector<Xchg_VertexPtr> verts;
                verts.push_back(xv);
                xchg_shell->AddWire(verts);
            }
        }
    }

    Xchg_bool is_closed = xchg_shell->CheckIfClosed();
    xchg_shell->SetClosedInfo(is_closed);

    *shell = xchg_shell;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Face (with dedup)
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertFace(PK_FACE_t pk_face, Xchg_FacePtr* face)
{
    *face = nullptr;

    auto it = pk_face_map_.find(pk_face);
    if (it != pk_face_map_.end()) {
        *face = it->second;
        return STEP_OK;
    }

    Xchg_FacePtr xchg_face = Xchg_Face::Create(current_body_);

    PK_SURF_t pk_surf = PK_ENTITY_null;
    PK_LOGICAL_t surf_orient = PK_LOGICAL_true;
    STEPExport_ErrorCode rc = PKErr(PK_FACE_ask_oriented_surf(pk_face, &pk_surf, &surf_orient),
                              "PK_FACE_ask_oriented_surf");
    if (rc != STEP_OK)
        return rc;

    if (pk_surf != PK_ENTITY_null) {
        Xchg_SurfacePtr xchg_surface;
        rc = ConvertSurface(pk_surf, &xchg_surface);
        if (rc != STEP_OK)
            return rc;
        if (xchg_surface) {
            xchg_face->SetGeom(xchg_surface);
            if (surf_orient == PK_LOGICAL_false) {
                // TODO: reverse the surface parameterization or use ReverseNormal
            }
        }
    }

    int n_loops = 0;
    PK_LOOP_t* loops = nullptr;
    rc = PKErr(PK_FACE_ask_loops(pk_face, &n_loops, &loops), "PK_FACE_ask_loops");
    if (rc != STEP_OK)
        return rc;
    PKMemGuard loops_guard(loops);

    for (int i = 0; i < n_loops; ++i) {
        PK_LOOP_type_t loop_type = PK_LOOP_type_outer_c;
        rc = PKErr(PK_LOOP_ask_type(loops[i], &loop_type), "PK_LOOP_ask_type");
        if (rc != STEP_OK)
            return rc;

        if (loop_type == PK_LOOP_type_vertex_c) {
            Xchg_LoopPtr xchg_loop;
            rc = ConvertLoop(loops[i], &xchg_loop);
            if (rc != STEP_OK)
                return rc;
            if (xchg_loop)
                xchg_face->AddLoop(xchg_loop, XCHG_FALSE);
            continue;
        }

        Xchg_LoopPtr xchg_loop;
        rc = ConvertLoop(loops[i], &xchg_loop);
        if (rc != STEP_OK)
            return rc;
        if (!xchg_loop)
            continue;

        switch (loop_type) {
        case PK_LOOP_type_outer_c:
        case PK_LOOP_type_likely_outer_c:
            xchg_face->AddOuterLoop(xchg_loop);
            break;
        case PK_LOOP_type_inner_c:
        case PK_LOOP_type_likely_inner_c:
        case PK_LOOP_type_inner_sing_c:
            xchg_face->AddInnerLoop(xchg_loop);
            break;
        case PK_LOOP_type_winding_c:
            xchg_face->AddLoop(xchg_loop, XCHG_FALSE);
            break;
        case PK_LOOP_type_wire_c:
            xchg_face->AddInnerLoop(xchg_loop);
            break;
        default:
            xchg_face->AddLoop(xchg_loop, XCHG_FALSE);
            break;
        }
    }

    pk_face_map_[pk_face] = xchg_face;
    *face = xchg_face;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertLoop(PK_LOOP_t pk_loop, Xchg_LoopPtr* loop)
{
    *loop = nullptr;

    Xchg_LoopPtr xchg_loop = Xchg_Loop::Create(current_body_);

    PK_LOOP_type_t loop_type = PK_LOOP_type_outer_c;
    STEPExport_ErrorCode rc = PKErr(PK_LOOP_ask_type(pk_loop, &loop_type), "PK_LOOP_ask_type");
    if (rc != STEP_OK)
        return rc;

    bool is_outer = (loop_type == PK_LOOP_type_outer_c ||
                     loop_type == PK_LOOP_type_likely_outer_c);
    xchg_loop->SetOuterInfo(is_outer ? XCHG_TRUE : XCHG_FALSE);
    xchg_loop->SetOrientation(XCHG_TRUE);

    int n_fins = 0;
    PK_FIN_t* fins = nullptr;
    rc = PKErr(PK_LOOP_ask_fins(pk_loop, &n_fins, &fins), "PK_LOOP_ask_fins");
    if (rc != STEP_OK)
        return rc;
    PKMemGuard fins_guard(fins);

    for (int i = 0; i < n_fins; ++i) {
        Xchg_CoedgePtr xchg_coedge;
        rc = ConvertFin(fins[i], &xchg_coedge);
        if (rc != STEP_OK)
            return rc;
        if (!xchg_coedge)
            continue;

        // AddCoedge 第二个参数: coedge 在 loop 中的方向是否与 loop 一致。
        // PK 中 fin 按 nose-to-tail 排列，fin 方向就是 loop 方向，所以总是 XCHG_TRUE。
        // coedge 与 edge 的方向关系已在 ConvertFin 中通过 SetOrientation 设置。
        xchg_loop->AddCoedge(xchg_coedge, XCHG_TRUE);
    }

    *loop = xchg_loop;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Fin -> Coedge
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertFin(PK_FIN_t pk_fin, Xchg_CoedgePtr* coedge)
{
    *coedge = nullptr;

    Xchg_CoedgePtr xchg_coedge = Xchg_Coedge::Create(current_body_);

    PK_EDGE_t pk_edge = PK_ENTITY_null;
    STEPExport_ErrorCode rc = PKErr(PK_FIN_ask_edge(pk_fin, &pk_edge), "PK_FIN_ask_edge");
    if (rc != STEP_OK)
        return rc;

    Xchg_EdgePtr xchg_edge;
    rc = ConvertEdge(pk_edge, &xchg_edge);
    if (rc != STEP_OK)
        return rc;

    if (xchg_edge) {
        xchg_coedge->SetEdge(xchg_edge);
        xchg_edge->AddCoedge(xchg_coedge);
    }

    PK_LOGICAL_t is_positive = PK_LOGICAL_true;
    rc = PKErr(PK_FIN_is_positive(pk_fin, &is_positive), "PK_FIN_is_positive");
    if (rc != STEP_OK)
        return rc;
    xchg_coedge->SetOrientation(is_positive ? XCHG_TRUE : XCHG_FALSE);

    PK_CURVE_t fin_curve = PK_ENTITY_null;
    PK_LOGICAL_t fin_orient = PK_LOGICAL_true;
    rc = PKErr(PK_FIN_ask_oriented_curve(pk_fin, &fin_curve, &fin_orient),
               "PK_FIN_ask_oriented_curve");
    if (rc != STEP_OK)
        return rc;
    if (fin_curve != PK_ENTITY_null) {
        Xchg_CurvePtr uv_curve;
        rc = ConvertCurve(fin_curve, &uv_curve);
        if (rc != STEP_OK)
            return rc;
        if (uv_curve)
            xchg_coedge->SetGeom(uv_curve);
    }

    *coedge = xchg_coedge;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Edge (with dedup)
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertEdge(PK_EDGE_t pk_edge, Xchg_EdgePtr* edge)
{
    *edge = nullptr;

    auto it = pk_edge_map_.find(pk_edge);
    if (it != pk_edge_map_.end()) {
        *edge = it->second;
        return STEP_OK;
    }

    Xchg_EdgePtr xchg_edge = Xchg_Edge::Create(current_body_);

    PK_VERTEX_t vertices[2] = { PK_ENTITY_null, PK_ENTITY_null };
    STEPExport_ErrorCode rc = PKErr(PK_EDGE_ask_vertices(pk_edge, vertices), "PK_EDGE_ask_vertices");
    if (rc != STEP_OK)
        return rc;

    if (vertices[0] != PK_ENTITY_null) {
        Xchg_VertexPtr sv;
        rc = ConvertVertex(vertices[0], &sv);
        if (rc != STEP_OK)
            return rc;
        if (sv) xchg_edge->SetStartVertex(sv);
    }
    if (vertices[1] != PK_ENTITY_null) {
        Xchg_VertexPtr ev;
        rc = ConvertVertex(vertices[1], &ev);
        if (rc != STEP_OK)
            return rc;
        if (ev) xchg_edge->SetEndVertex(ev);
    }

    PK_CURVE_t pk_curve = PK_ENTITY_null;
    rc = PKErr(PK_EDGE_ask_curve(pk_edge, &pk_curve), "PK_EDGE_ask_curve");
    if (rc != STEP_OK)
        return rc;

    if (pk_curve != PK_ENTITY_null) {
        Xchg_CurvePtr xchg_curve;
        rc = ConvertCurve(pk_curve, &xchg_curve);
        if (rc != STEP_OK)
            return rc;
        if (xchg_curve)
            xchg_edge->SetGeom(xchg_curve);

        PK_CURVE_t oriented_curve = PK_ENTITY_null;
        PK_LOGICAL_t orient = PK_LOGICAL_true;
        rc = PKErr(PK_EDGE_ask_oriented_curve(pk_edge, &oriented_curve, &orient),
                   "PK_EDGE_ask_oriented_curve");
        if (rc != STEP_OK)
            return rc;
        xchg_edge->SetSameSense(orient ? XCHG_TRUE : XCHG_FALSE);
    } else {
        xchg_edge->SetSameSense(XCHG_TRUE);
    }

    bool degenerated = (pk_curve == PK_ENTITY_null) ||
                       (vertices[0] != PK_ENTITY_null && vertices[0] == vertices[1]);
    if (degenerated)
        xchg_edge->SetDegenerated(XCHG_TRUE);

    pk_edge_map_[pk_edge] = xchg_edge;
    *edge = xchg_edge;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Vertex (with dedup)
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertVertex(PK_VERTEX_t pk_vertex, Xchg_VertexPtr* vertex)
{
    *vertex = nullptr;

    auto it = pk_vertex_map_.find(pk_vertex);
    if (it != pk_vertex_map_.end()) {
        *vertex = it->second;
        return STEP_OK;
    }

    Xchg_VertexPtr xchg_vertex = Xchg_Vertex::Create(current_body_);

    PK_POINT_t pk_point = PK_ENTITY_null;
    STEPExport_ErrorCode rc = PKErr(PK_VERTEX_ask_point(pk_vertex, &pk_point), "PK_VERTEX_ask_point");
    if (rc != STEP_OK)
        return rc;

    if (pk_point != PK_ENTITY_null) {
        Xchg_PointPtr xchg_point;
        rc = ConvertPoint(pk_point, &xchg_point);
        if (rc != STEP_OK)
            return rc;
        if (xchg_point)
            xchg_vertex->SetGeom(xchg_point);
    }

    pk_vertex_map_[pk_vertex] = xchg_vertex;
    *vertex = xchg_vertex;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Geometry: Point
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertPoint(PK_POINT_t pk_point, Xchg_PointPtr* point)
{
    *point = nullptr;

    PK_POINT_sf_t point_sf;
    STEPExport_ErrorCode rc = PKErr(PK_POINT_ask(pk_point, &point_sf), "PK_POINT_ask");
    if (rc != STEP_OK)
        return rc;

    *point = Xchg_Point::Create(
        point_sf.position.coord[0],
        point_sf.position.coord[1],
        point_sf.position.coord[2]);
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Geometry: Surface dispatch
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    auto it = pk_surface_map_.find(pk_surf);
    if (it != pk_surface_map_.end()) {
        *surface = it->second;
        return STEP_OK;
    }

    PK_CLASS_t surf_class = PK_CLASS_null;
    STEPExport_ErrorCode rc = PKErr(PK_ENTITY_ask_class(pk_surf, &surf_class), "PK_ENTITY_ask_class(surface)");
    if (rc != STEP_OK)
        return rc;

    Xchg_SurfacePtr result;

    if (surf_class == PK_CLASS_plane)
        rc = ConvertPlaneSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_cyl)
        rc = ConvertCylindricalSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_cone)
        rc = ConvertConicalSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_sphere)
        rc = ConvertSphericalSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_torus)
        rc = ConvertToroidalSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_bsurf)
        rc = ConvertNurbsSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_swept)
        rc = ConvertSweptSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_spun)
        rc = ConvertSpunSurface(pk_surf, &result);
    else if (surf_class == PK_CLASS_offset)
        rc = ConvertOffsetSurface(pk_surf, &result);
    else {
        Log("Unsupported surface class");
        return STEP_OK;
    }

    if (rc != STEP_OK)
        return rc;

    if (result)
        pk_surface_map_[pk_surf] = result;

    *surface = result;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Geometry: Curve dispatch
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve)
{
    *curve = nullptr;

    auto it = pk_curve_map_.find(pk_curve);
    if (it != pk_curve_map_.end()) {
        *curve = it->second;
        return STEP_OK;
    }

    PK_CLASS_t curve_class = PK_CLASS_null;
    STEPExport_ErrorCode rc = PKErr(PK_ENTITY_ask_class(pk_curve, &curve_class), "PK_ENTITY_ask_class(curve)");
    if (rc != STEP_OK)
        return rc;

    Xchg_CurvePtr result;

    if (curve_class == PK_CLASS_line)
        rc = ConvertLineCurve(pk_curve, &result);
    else if (curve_class == PK_CLASS_circle)
        rc = ConvertCircleCurve(pk_curve, &result);
    else if (curve_class == PK_CLASS_ellipse)
        rc = ConvertEllipseCurve(pk_curve, &result);
    else if (curve_class == PK_CLASS_bcurve)
        rc = ConvertNurbsCurve(pk_curve, &result);
    else {
        Log("Unsupported curve class");
        return STEP_OK;
    }

    if (rc != STEP_OK)
        return rc;

    if (result)
        pk_curve_map_[pk_curve] = result;

    *curve = result;
    return STEP_OK;
}

// ---------------------------------------------------------------------------
// Geometry stubs (to be implemented)
// ---------------------------------------------------------------------------

STEPExport_ErrorCode PKToXchgConverter::ConvertPlaneSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_PLANE_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_PLANE_ask(pk_surf, &sf), "PK_PLANE_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt origin(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir normal(sf.basis_set.axis.coord[0],
                    sf.basis_set.axis.coord[1],
                    sf.basis_set.axis.coord[2]);
    Xchg_dir u_dir(sf.basis_set.ref_direction.coord[0],
                   sf.basis_set.ref_direction.coord[1],
                   sf.basis_set.ref_direction.coord[2]);

    auto plane = Xchg_PlaneSurface::Create(origin, normal, u_dir);
    *surface = Xchg_SurfacePtr(plane.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertCylindricalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_CYL_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_CYL_ask(pk_surf, &sf), "PK_CYL_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt origin(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir axis(sf.basis_set.axis.coord[0],
                  sf.basis_set.axis.coord[1],
                  sf.basis_set.axis.coord[2]);
    Xchg_dir u_dir(sf.basis_set.ref_direction.coord[0],
                   sf.basis_set.ref_direction.coord[1],
                   sf.basis_set.ref_direction.coord[2]);

    auto cyl = Xchg_CylindricalSurface::Create(origin, axis, u_dir, sf.radius);
    *surface = Xchg_SurfacePtr(cyl.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertConicalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_CONE_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_CONE_ask(pk_surf, &sf), "PK_CONE_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt origin(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir axis(sf.basis_set.axis.coord[0],
                  sf.basis_set.axis.coord[1],
                  sf.basis_set.axis.coord[2]);
    Xchg_dir u_dir(sf.basis_set.ref_direction.coord[0],
                   sf.basis_set.ref_direction.coord[1],
                   sf.basis_set.ref_direction.coord[2]);

    auto cone = Xchg_ConicalSurface::Create(origin, axis, u_dir, sf.radius, sf.semi_angle);
    *surface = Xchg_SurfacePtr(cone.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertSphericalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_SPHERE_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_SPHERE_ask(pk_surf, &sf), "PK_SPHERE_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt origin(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir axis(sf.basis_set.axis.coord[0],
                  sf.basis_set.axis.coord[1],
                  sf.basis_set.axis.coord[2]);
    Xchg_dir u_dir(sf.basis_set.ref_direction.coord[0],
                   sf.basis_set.ref_direction.coord[1],
                   sf.basis_set.ref_direction.coord[2]);

    auto sphere = Xchg_SphericalSurface::Create(origin, axis, u_dir, sf.radius);
    *surface = Xchg_SurfacePtr(sphere.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertToroidalSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_TORUS_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_TORUS_ask(pk_surf, &sf), "PK_TORUS_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt origin(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir axis(sf.basis_set.axis.coord[0],
                  sf.basis_set.axis.coord[1],
                  sf.basis_set.axis.coord[2]);
    Xchg_dir u_dir(sf.basis_set.ref_direction.coord[0],
                   sf.basis_set.ref_direction.coord[1],
                   sf.basis_set.ref_direction.coord[2]);

    auto torus = Xchg_ToroidalSurface::Create(origin, axis, u_dir, sf.major_radius, sf.minor_radius);
    *surface = Xchg_SurfacePtr(torus.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertNurbsSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_BSURF_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_BSURF_ask(pk_surf, &sf), "PK_BSURF_ask");
    if (rc != STEP_OK)
        return rc;

    // U knots
    Xchg_vector<Xchg_Double64> u_knots;
    Xchg_vector<Xchg_UChar8>   u_mults;
    for (int i = 0; i < sf.n_u_knots; ++i) {
        u_knots.push_back(sf.u_knot[i]);
        u_mults.push_back(static_cast<Xchg_UChar8>(sf.u_knot_mult[i]));
    }

    // V knots
    Xchg_vector<Xchg_Double64> v_knots;
    Xchg_vector<Xchg_UChar8>   v_mults;
    for (int i = 0; i < sf.n_v_knots; ++i) {
        v_knots.push_back(sf.v_knot[i]);
        v_mults.push_back(static_cast<Xchg_UChar8>(sf.v_knot_mult[i]));
    }

    // Control points (PK stores in v-major order) and weights
    int dim = sf.vertex_dim;  // 3 = polynomial, 4 = rational
    int n_pts = sf.n_u_vertices * sf.n_v_vertices;
    Xchg_vector<Xchg_pnt>     ctrl_pts;
    Xchg_vector<Xchg_Double64> weights;

    for (int i = 0; i < n_pts; ++i) {
        const double* v = sf.vertex + i * dim;
        if (sf.is_rational && dim == 4) {
            double w = v[3];
            if (w != 0.0)
                ctrl_pts.push_back(Xchg_pnt(v[0] / w, v[1] / w, v[2] / w));
            else
                ctrl_pts.push_back(Xchg_pnt(v[0], v[1], v[2]));
            weights.push_back(w);
        } else {
            ctrl_pts.push_back(Xchg_pnt(v[0], v[1], v[2]));
        }
    }

    Xchg_bool u_closed = sf.is_u_closed ? XCHG_TRUE : XCHG_FALSE;
    Xchg_bool v_closed = sf.is_v_closed ? XCHG_TRUE : XCHG_FALSE;

    Xchg_NurbsSurfacePtr nurbs = Xchg_NurbsSurface::Create(
        static_cast<Xchg_UInt32>(sf.u_degree),
        static_cast<Xchg_UInt32>(sf.v_degree),
        u_knots, v_knots, u_mults, v_mults,
        ctrl_pts, u_closed, v_closed, weights);

    if (nurbs) {
        if (sf.is_u_periodic)
            nurbs->SetUPeriodic();
        if (sf.is_v_periodic)
            nurbs->SetVPeriodic();
    }

    // Free PK-allocated memory inside sf
    if (sf.vertex)      PK_MEMORY_free(sf.vertex);
    if (sf.u_knot)      PK_MEMORY_free(sf.u_knot);
    if (sf.v_knot)      PK_MEMORY_free(sf.v_knot);
    if (sf.u_knot_mult) PK_MEMORY_free(sf.u_knot_mult);
    if (sf.v_knot_mult) PK_MEMORY_free(sf.v_knot_mult);

    *surface = Xchg_SurfacePtr(nurbs.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertSweptSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_SWEPT_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_SWEPT_ask(pk_surf, &sf), "PK_SWEPT_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_CurvePtr profile;
    rc = ConvertCurve(sf.curve, &profile);
    if (rc != STEP_OK)
        return rc;
    if (!profile) {
        Log("SweptSurface: failed to convert profile curve");
        return STEP_OK;
    }

    Xchg_dir extrusion_dir(sf.direction.coord[0],
                           sf.direction.coord[1],
                           sf.direction.coord[2]);

    auto swept = Xchg_LinearExtrusionSurface::Create(profile, extrusion_dir);
    *surface = Xchg_SurfacePtr(swept.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertSpunSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_SPUN_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_SPUN_ask(pk_surf, &sf), "PK_SPUN_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_CurvePtr meridian;
    rc = ConvertCurve(sf.curve, &meridian);
    if (rc != STEP_OK)
        return rc;
    if (!meridian) {
        Log("SpunSurface: failed to convert meridian curve");
        return STEP_OK;
    }

    Xchg_pnt axis_pos(sf.axis.location.coord[0],
                      sf.axis.location.coord[1],
                      sf.axis.location.coord[2]);
    Xchg_dir axis_dir(sf.axis.axis.coord[0],
                      sf.axis.axis.coord[1],
                      sf.axis.axis.coord[2]);

    auto rev = Xchg_RevolutionSurface::Create(meridian, axis_pos, axis_dir);
    *surface = Xchg_SurfacePtr(rev.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertOffsetSurface(PK_SURF_t pk_surf, Xchg_SurfacePtr* surface)
{
    *surface = nullptr;

    PK_OFFSET_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_OFFSET_ask(pk_surf, &sf), "PK_OFFSET_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_SurfacePtr base_surf;
    rc = ConvertSurface(sf.underlying_surface, &base_surf);
    if (rc != STEP_OK)
        return rc;
    if (!base_surf) {
        Log("OffsetSurface: failed to convert underlying surface");
        return STEP_OK;
    }

    auto offset = Xchg_OffsetSurface::Create(base_surf, sf.offset_distance);
    *surface = Xchg_SurfacePtr(offset.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertLineCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve)
{
    *curve = nullptr;

    PK_LINE_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_LINE_ask(pk_curve, &sf), "PK_LINE_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt origin(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir dir(sf.basis_set.axis.coord[0],
                 sf.basis_set.axis.coord[1],
                 sf.basis_set.axis.coord[2]);

    auto line = Xchg_Line::Create(origin, dir);
    *curve = Xchg_CurvePtr(line.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertCircleCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve)
{
    *curve = nullptr;

    PK_CIRCLE_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_CIRCLE_ask(pk_curve, &sf), "PK_CIRCLE_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt center(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir normal(sf.basis_set.axis.coord[0],
                    sf.basis_set.axis.coord[1],
                    sf.basis_set.axis.coord[2]);
    Xchg_dir x_ref(sf.basis_set.ref_direction.coord[0],
                   sf.basis_set.ref_direction.coord[1],
                   sf.basis_set.ref_direction.coord[2]);

    auto circle = Xchg_Ellipse::Create(center, normal, x_ref, sf.radius);
    *curve = Xchg_CurvePtr(circle.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertEllipseCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve)
{
    *curve = nullptr;

    PK_ELLIPSE_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_ELLIPSE_ask(pk_curve, &sf), "PK_ELLIPSE_ask");
    if (rc != STEP_OK)
        return rc;

    Xchg_pnt center(sf.basis_set.location.coord[0],
                    sf.basis_set.location.coord[1],
                    sf.basis_set.location.coord[2]);
    Xchg_dir normal(sf.basis_set.axis.coord[0],
                    sf.basis_set.axis.coord[1],
                    sf.basis_set.axis.coord[2]);
    Xchg_dir x_ref(sf.basis_set.ref_direction.coord[0],
                   sf.basis_set.ref_direction.coord[1],
                   sf.basis_set.ref_direction.coord[2]);

    auto ellipse = Xchg_Ellipse::Create(center, normal, x_ref, sf.R1, sf.R2);
    *curve = Xchg_CurvePtr(ellipse.get());
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertNurbsCurve(PK_CURVE_t pk_curve, Xchg_CurvePtr* curve)
{
    *curve = nullptr;

    PK_BCURVE_sf_t sf;
    STEPExport_ErrorCode rc = PKErr(PK_BCURVE_ask(pk_curve, &sf), "PK_BCURVE_ask");
    if (rc != STEP_OK)
        return rc;

    // Build flat knot vector from PK's (value, multiplicity) pairs
    Xchg_vector<Xchg_Double64> knots;
    Xchg_vector<Xchg_UChar8>   mults;
    for (int i = 0; i < sf.n_knots; ++i) {
        knots.push_back(sf.knot[i]);
        mults.push_back(static_cast<Xchg_UChar8>(sf.knot_mult[i]));
    }

    // Extract control points and weights
    int dim = sf.vertex_dim;  // 3 = polynomial 3D, 4 = rational 3D, 2 = UV curve
    Xchg_vector<Xchg_pnt>     ctrl_pts;
    Xchg_vector<Xchg_Double64> weights;

    for (int i = 0; i < sf.n_vertices; ++i) {
        const double* v = sf.vertex + i * dim;
        if (sf.is_rational && dim == 4) {
            double w = v[3];
            if (w != 0.0)
                ctrl_pts.push_back(Xchg_pnt(v[0] / w, v[1] / w, v[2] / w));
            else
                ctrl_pts.push_back(Xchg_pnt(v[0], v[1], v[2]));
            weights.push_back(w);
        } else if (dim == 3) {
            ctrl_pts.push_back(Xchg_pnt(v[0], v[1], v[2]));
        } else if (dim == 2) {
            ctrl_pts.push_back(Xchg_pnt(v[0], v[1], 0.0));
        }
    }

    Xchg_NurbsCurvePtr nurbs = Xchg_NurbsCurve::Create(
        static_cast<Xchg_UInt32>(sf.degree),
        knots, mults, ctrl_pts, weights);

    if (nurbs) {
        if (sf.is_periodic)
            nurbs->SetPeriodic();
        if (sf.is_closed)
            nurbs->SetClosed(XCHG_TRUE);
    }

    // Free PK-allocated memory inside sf
    if (sf.vertex)    PK_MEMORY_free(sf.vertex);
    if (sf.knot)      PK_MEMORY_free(sf.knot);
    if (sf.knot_mult) PK_MEMORY_free(sf.knot_mult);

    *curve = Xchg_CurvePtr(nurbs.get());
    return STEP_OK;
}
