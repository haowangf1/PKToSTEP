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

        PK_LOGICAL_t is_positive = PK_LOGICAL_true;
        rc = PKErr(PK_FIN_is_positive(fins[i], &is_positive), "PK_FIN_is_positive");
        if (rc != STEP_OK)
            return rc;

        xchg_loop->AddCoedge(xchg_coedge, is_positive ? XCHG_TRUE : XCHG_FALSE);
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

STEPExport_ErrorCode PKToXchgConverter::ConvertPlaneSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertCylindricalSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertConicalSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertSphericalSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertToroidalSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertNurbsSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertSweptSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertSpunSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertOffsetSurface(PK_SURF_t /*pk_surf*/, Xchg_SurfacePtr* surface)
{
    // TODO: implement
    *surface = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertLineCurve(PK_CURVE_t /*pk_curve*/, Xchg_CurvePtr* curve)
{
    // TODO: implement
    *curve = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertCircleCurve(PK_CURVE_t /*pk_curve*/, Xchg_CurvePtr* curve)
{
    // TODO: implement
    *curve = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertEllipseCurve(PK_CURVE_t /*pk_curve*/, Xchg_CurvePtr* curve)
{
    // TODO: implement
    *curve = nullptr;
    return STEP_OK;
}

STEPExport_ErrorCode PKToXchgConverter::ConvertNurbsCurve(PK_CURVE_t /*pk_curve*/, Xchg_CurvePtr* curve)
{
    // TODO: implement
    *curve = nullptr;
    return STEP_OK;
}
