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

bool PKToXchgConverter::PKCheck(PK_ERROR_code_t err, const char* context)
{
    if (err != PK_ERROR_no_errors) {
        char buf[256];
        snprintf(buf, sizeof(buf), "PK error %d at %s", static_cast<int>(err), context);
        Log(buf);
        return true;
    }
    return false;
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

Xchg_BodyPtr PKToXchgConverter::Convert(PK_BODY_t pk_body)
{
    ClearMaps();

    PK_BODY_type_t body_type = PK_BODY_type_solid_c;
    if (PKCheck(PK_BODY_ask_type(pk_body, &body_type), "PK_BODY_ask_type"))
        return nullptr;

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

    ConvertRegions(pk_body, body_type);

    return current_body_;
}

// ---------------------------------------------------------------------------
// Region -> Lump (all regions, no skipping)
// ---------------------------------------------------------------------------

void PKToXchgConverter::ConvertRegions(PK_BODY_t pk_body, PK_BODY_type_t body_type)
{
    int n_regions = 0;
    PK_REGION_t* regions = nullptr;
    if (PKCheck(PK_BODY_ask_regions(pk_body, &n_regions, &regions), "PK_BODY_ask_regions"))
        return;
    PKMemGuard regions_guard(regions);

    for (int ri = 0; ri < n_regions; ++ri) {
        PK_REGION_t region = regions[ri];

        PK_LOGICAL_t is_solid = PK_LOGICAL_false;
        if (PKCheck(PK_REGION_is_solid(region, &is_solid), "PK_REGION_is_solid"))
            continue;

        bool is_infinite_void = (ri == 0 && is_solid == PK_LOGICAL_false);

        Xchg_LumpPtr lump = Xchg_Lump::Create(current_body_);
        current_body_->AddLump(lump);

        int n_shells = 0;
        PK_SHELL_t* shells = nullptr;
        if (PKCheck(PK_REGION_ask_shells(region, &n_shells, &shells), "PK_REGION_ask_shells"))
            continue;
        PKMemGuard shells_guard(shells);

        for (int si = 0; si < n_shells; ++si) {
            PK_SHELL_type_t shell_type = PK_SHELL_type_wireframe_free_c;
            PK_SHELL_ask_type(shells[si], &shell_type);

            Xchg_ShellPtr xchg_shell = ConvertShell(shells[si]);
            if (!xchg_shell)
                continue;

            AddShellToLump(xchg_shell, lump,
                           is_solid == PK_LOGICAL_true,
                           is_infinite_void,
                           body_type, shell_type);
        }
    }
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
        // bounded void region (cavity)
        lump->AddInnerShell(shell);
    }
}

// ---------------------------------------------------------------------------
// Shell
// ---------------------------------------------------------------------------

Xchg_ShellPtr PKToXchgConverter::ConvertShell(PK_SHELL_t pk_shell)
{
    PK_SHELL_type_t shell_type = PK_SHELL_type_wireframe_free_c;
    PK_SHELL_ask_type(pk_shell, &shell_type);

    Xchg_ShellPtr xchg_shell = Xchg_Shell::Create(current_body_);

    if (shell_type == PK_SHELL_type_wireframe_free_c ||
        shell_type == PK_SHELL_type_mixed_c) {
        int n_faces = 0;
        PK_FACE_t* faces = nullptr;
        PK_LOGICAL_t* orients = nullptr;
        if (PKCheck(PK_SHELL_ask_oriented_faces(pk_shell, &n_faces, &faces, &orients),
                    "PK_SHELL_ask_oriented_faces"))
            return xchg_shell;
        PKMemGuard faces_guard(faces);
        PKMemGuard orients_guard(orients);

        for (int i = 0; i < n_faces; ++i) {
            Xchg_FacePtr xchg_face = ConvertFace(faces[i]);
            if (!xchg_face)
                continue;

            PK_SURF_t surf = PK_ENTITY_null;
            PK_LOGICAL_t face_orient = PK_LOGICAL_true;
            PK_FACE_ask_oriented_surf(faces[i], &surf, &face_orient);

            // xchg_shell_orient = (pk_shell_orient == pk_face_orient)
            bool same = (orients[i] == face_orient);
            xchg_shell->AddFace(xchg_face, same ? XCHG_TRUE : XCHG_FALSE);
        }
    }

    if (shell_type == PK_SHELL_type_wireframe_c ||
        shell_type == PK_SHELL_type_mixed_c) {
        int n_edges = 0;
        PK_EDGE_t* edges = nullptr;
        PK_SHELL_ask_wireframe_edges(pk_shell, &n_edges, &edges);
        PKMemGuard edges_guard(edges);

        Xchg_vector<Xchg_EdgePtr> xchg_edges;
        for (int i = 0; i < n_edges; ++i) {
            Xchg_EdgePtr e = ConvertEdge(edges[i]);
            if (e)
                xchg_edges.push_back(e);
        }
        if (!xchg_edges.empty())
            xchg_shell->AddWire(xchg_edges);
    }

    if (shell_type == PK_SHELL_type_acorn_c) {
        PK_VERTEX_t vtx = PK_ENTITY_null;
        PK_SHELL_ask_acorn_vertex(pk_shell, &vtx);
        if (vtx != PK_ENTITY_null) {
            Xchg_VertexPtr xv = ConvertVertex(vtx);
            if (xv) {
                Xchg_vector<Xchg_VertexPtr> verts;
                verts.push_back(xv);
                xchg_shell->AddWire(verts);
            }
        }
    }

    Xchg_bool is_closed = xchg_shell->CheckIfClosed();
    xchg_shell->SetClosedInfo(is_closed);

    return xchg_shell;
}

// ---------------------------------------------------------------------------
// Face (with dedup)
// ---------------------------------------------------------------------------

Xchg_FacePtr PKToXchgConverter::ConvertFace(PK_FACE_t pk_face)
{
    auto it = pk_face_map_.find(pk_face);
    if (it != pk_face_map_.end())
        return it->second;

    Xchg_FacePtr xchg_face = Xchg_Face::Create(current_body_);

    PK_SURF_t pk_surf = PK_ENTITY_null;
    PK_LOGICAL_t surf_orient = PK_LOGICAL_true;
    PK_FACE_ask_oriented_surf(pk_face, &pk_surf, &surf_orient);

    if (pk_surf != PK_ENTITY_null) {
        Xchg_SurfacePtr xchg_surface = ConvertSurface(pk_surf);
        if (xchg_surface) {
            xchg_face->SetGeom(xchg_surface);
            if (surf_orient == PK_LOGICAL_false) {
                // TODO: reverse the surface parameterization or use ReverseNormal
                // For now we set the surface as-is; orientation is captured in shell-face orient
            }
        }
    }

    int n_loops = 0;
    PK_LOOP_t* loops = nullptr;
    if (!PKCheck(PK_FACE_ask_loops(pk_face, &n_loops, &loops), "PK_FACE_ask_loops")) {
        PKMemGuard loops_guard(loops);

        for (int i = 0; i < n_loops; ++i) {
            PK_LOOP_type_t loop_type = PK_LOOP_type_outer_c;
            PK_LOOP_ask_type(loops[i], &loop_type);

            if (loop_type == PK_LOOP_type_vertex_c) {
                // vertex loop: get the vertex directly
                int n_fins = 0;
                PK_FIN_t* fins = nullptr;
                PK_LOOP_ask_fins(loops[i], &n_fins, &fins);
                PKMemGuard fins_guard(fins);
                // vertex loop has no fins; get vertex via loop's vertices
                // PK doesn't have PK_LOOP_ask_vertices, use topology traversal
                // For now create an empty loop and mark it as vertex
                Xchg_LoopPtr xchg_loop = ConvertLoop(loops[i]);
                if (xchg_loop)
                    xchg_face->AddLoop(xchg_loop, XCHG_FALSE);
                continue;
            }

            Xchg_LoopPtr xchg_loop = ConvertLoop(loops[i]);
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
    }

    pk_face_map_[pk_face] = xchg_face;
    return xchg_face;
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------

Xchg_LoopPtr PKToXchgConverter::ConvertLoop(PK_LOOP_t pk_loop)
{
    Xchg_LoopPtr xchg_loop = Xchg_Loop::Create(current_body_);

    PK_LOOP_type_t loop_type = PK_LOOP_type_outer_c;
    PK_LOOP_ask_type(pk_loop, &loop_type);

    bool is_outer = (loop_type == PK_LOOP_type_outer_c ||
                     loop_type == PK_LOOP_type_likely_outer_c);
    xchg_loop->SetOuterInfo(is_outer ? XCHG_TRUE : XCHG_FALSE);
    xchg_loop->SetOrientation(XCHG_TRUE);

    int n_fins = 0;
    PK_FIN_t* fins = nullptr;
    if (PKCheck(PK_LOOP_ask_fins(pk_loop, &n_fins, &fins), "PK_LOOP_ask_fins"))
        return xchg_loop;
    PKMemGuard fins_guard(fins);

    for (int i = 0; i < n_fins; ++i) {
        Xchg_CoedgePtr xchg_coedge = ConvertFin(fins[i]);
        if (!xchg_coedge)
            continue;

        PK_LOGICAL_t is_positive = PK_LOGICAL_true;
        PK_FIN_is_positive(fins[i], &is_positive);

        xchg_loop->AddCoedge(xchg_coedge, is_positive ? XCHG_TRUE : XCHG_FALSE);
    }

    return xchg_loop;
}

// ---------------------------------------------------------------------------
// Fin -> Coedge
// ---------------------------------------------------------------------------

Xchg_CoedgePtr PKToXchgConverter::ConvertFin(PK_FIN_t pk_fin)
{
    Xchg_CoedgePtr xchg_coedge = Xchg_Coedge::Create(current_body_);

    PK_EDGE_t pk_edge = PK_ENTITY_null;
    if (PKCheck(PK_FIN_ask_edge(pk_fin, &pk_edge), "PK_FIN_ask_edge"))
        return xchg_coedge;

    Xchg_EdgePtr xchg_edge = ConvertEdge(pk_edge);
    if (xchg_edge) {
        xchg_coedge->SetEdge(xchg_edge);
        xchg_edge->AddCoedge(xchg_coedge);
    }

    PK_LOGICAL_t is_positive = PK_LOGICAL_true;
    PK_FIN_is_positive(pk_fin, &is_positive);
    xchg_coedge->SetOrientation(is_positive ? XCHG_TRUE : XCHG_FALSE);

    // SP-curve on fin (for tolerant edges)
    PK_CURVE_t fin_curve = PK_ENTITY_null;
    PK_LOGICAL_t fin_orient = PK_LOGICAL_true;
    PK_FIN_ask_oriented_curve(pk_fin, &fin_curve, &fin_orient);
    if (fin_curve != PK_ENTITY_null) {
        Xchg_CurvePtr uv_curve = ConvertCurve(fin_curve);
        if (uv_curve)
            xchg_coedge->SetGeom(uv_curve);
    }

    return xchg_coedge;
}

// ---------------------------------------------------------------------------
// Edge (with dedup)
// ---------------------------------------------------------------------------

Xchg_EdgePtr PKToXchgConverter::ConvertEdge(PK_EDGE_t pk_edge)
{
    auto it = pk_edge_map_.find(pk_edge);
    if (it != pk_edge_map_.end())
        return it->second;

    Xchg_EdgePtr xchg_edge = Xchg_Edge::Create(current_body_);

    PK_VERTEX_t vertices[2] = { PK_ENTITY_null, PK_ENTITY_null };
    PK_EDGE_ask_vertices(pk_edge, vertices);

    if (vertices[0] != PK_ENTITY_null) {
        Xchg_VertexPtr sv = ConvertVertex(vertices[0]);
        if (sv) xchg_edge->SetStartVertex(sv);
    }
    if (vertices[1] != PK_ENTITY_null) {
        Xchg_VertexPtr ev = ConvertVertex(vertices[1]);
        if (ev) xchg_edge->SetEndVertex(ev);
    }

    PK_CURVE_t pk_curve = PK_ENTITY_null;
    PK_EDGE_ask_curve(pk_edge, &pk_curve);

    if (pk_curve != PK_ENTITY_null) {
        Xchg_CurvePtr xchg_curve = ConvertCurve(pk_curve);
        if (xchg_curve)
            xchg_edge->SetGeom(xchg_curve);

        PK_CURVE_t oriented_curve = PK_ENTITY_null;
        PK_LOGICAL_t orient = PK_LOGICAL_true;
        PK_EDGE_ask_oriented_curve(pk_edge, &oriented_curve, &orient);
        xchg_edge->SetSameSense(orient ? XCHG_TRUE : XCHG_FALSE);
    } else {
        xchg_edge->SetSameSense(XCHG_TRUE);
    }

    bool degenerated = (pk_curve == PK_ENTITY_null) ||
                       (vertices[0] != PK_ENTITY_null && vertices[0] == vertices[1]);
    if (degenerated)
        xchg_edge->SetDegenerated(XCHG_TRUE);

    pk_edge_map_[pk_edge] = xchg_edge;
    return xchg_edge;
}

// ---------------------------------------------------------------------------
// Vertex (with dedup)
// ---------------------------------------------------------------------------

Xchg_VertexPtr PKToXchgConverter::ConvertVertex(PK_VERTEX_t pk_vertex)
{
    auto it = pk_vertex_map_.find(pk_vertex);
    if (it != pk_vertex_map_.end())
        return it->second;

    Xchg_VertexPtr xchg_vertex = Xchg_Vertex::Create(current_body_);

    PK_POINT_t pk_point = PK_ENTITY_null;
    PK_VERTEX_ask_point(pk_vertex, &pk_point);

    if (pk_point != PK_ENTITY_null) {
        Xchg_PointPtr xchg_point = ConvertPoint(pk_point);
        if (xchg_point)
            xchg_vertex->SetGeom(xchg_point);
    }

    pk_vertex_map_[pk_vertex] = xchg_vertex;
    return xchg_vertex;
}

// ---------------------------------------------------------------------------
// Geometry: Point
// ---------------------------------------------------------------------------

Xchg_PointPtr PKToXchgConverter::ConvertPoint(PK_POINT_t pk_point)
{
    PK_POINT_sf_t point_sf;
    if (PKCheck(PK_POINT_ask(pk_point, &point_sf), "PK_POINT_ask"))
        return nullptr;

    return Xchg_Point::Create(
        point_sf.position.coord[0],
        point_sf.position.coord[1],
        point_sf.position.coord[2]);
}

// ---------------------------------------------------------------------------
// Geometry: Surface dispatch
// ---------------------------------------------------------------------------

Xchg_SurfacePtr PKToXchgConverter::ConvertSurface(PK_SURF_t pk_surf)
{
    auto it = pk_surface_map_.find(pk_surf);
    if (it != pk_surface_map_.end())
        return it->second;

    PK_CLASS_t surf_class = PK_CLASS_null;
    PK_ENTITY_ask_class(pk_surf, &surf_class);

    Xchg_SurfacePtr result;

    if (surf_class == PK_CLASS_plane)
        result = ConvertPlaneSurface(pk_surf);
    else if (surf_class == PK_CLASS_cyl)
        result = ConvertCylindricalSurface(pk_surf);
    else if (surf_class == PK_CLASS_cone)
        result = ConvertConicalSurface(pk_surf);
    else if (surf_class == PK_CLASS_sphere)
        result = ConvertSphericalSurface(pk_surf);
    else if (surf_class == PK_CLASS_torus)
        result = ConvertToroidalSurface(pk_surf);
    else if (surf_class == PK_CLASS_bsurf)
        result = ConvertNurbsSurface(pk_surf);
    else if (surf_class == PK_CLASS_swept)
        result = ConvertSweptSurface(pk_surf);
    else if (surf_class == PK_CLASS_spun)
        result = ConvertSpunSurface(pk_surf);
    else if (surf_class == PK_CLASS_offset)
        result = ConvertOffsetSurface(pk_surf);
    else {
        Log("Unsupported surface class");
    }

    if (result)
        pk_surface_map_[pk_surf] = result;

    return result;
}

// ---------------------------------------------------------------------------
// Geometry: Curve dispatch
// ---------------------------------------------------------------------------

Xchg_CurvePtr PKToXchgConverter::ConvertCurve(PK_CURVE_t pk_curve)
{
    auto it = pk_curve_map_.find(pk_curve);
    if (it != pk_curve_map_.end())
        return it->second;

    PK_CLASS_t curve_class = PK_CLASS_null;
    PK_ENTITY_ask_class(pk_curve, &curve_class);

    Xchg_CurvePtr result;

    if (curve_class == PK_CLASS_line)
        result = ConvertLineCurve(pk_curve);
    else if (curve_class == PK_CLASS_circle)
        result = ConvertCircleCurve(pk_curve);
    else if (curve_class == PK_CLASS_ellipse)
        result = ConvertEllipseCurve(pk_curve);
    else if (curve_class == PK_CLASS_bcurve)
        result = ConvertNurbsCurve(pk_curve);
    else {
        Log("Unsupported curve class");
    }

    if (result)
        pk_curve_map_[pk_curve] = result;

    return result;
}

// ---------------------------------------------------------------------------
// Geometry stubs (to be implemented)
// ---------------------------------------------------------------------------

Xchg_SurfacePtr PKToXchgConverter::ConvertPlaneSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertCylindricalSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertConicalSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertSphericalSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertToroidalSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertNurbsSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertSweptSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertSpunSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_SurfacePtr PKToXchgConverter::ConvertOffsetSurface(PK_SURF_t /*pk_surf*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_CurvePtr PKToXchgConverter::ConvertLineCurve(PK_CURVE_t /*pk_curve*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_CurvePtr PKToXchgConverter::ConvertCircleCurve(PK_CURVE_t /*pk_curve*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_CurvePtr PKToXchgConverter::ConvertEllipseCurve(PK_CURVE_t /*pk_curve*/)
{
    // TODO: implement
    return nullptr;
}

Xchg_CurvePtr PKToXchgConverter::ConvertNurbsCurve(PK_CURVE_t /*pk_curve*/)
{
    // TODO: implement
    return nullptr;
}
