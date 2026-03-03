#pragma once

#include "topology/xchg_body.hpp"
#include "topology/xchg_lump.hpp"
#include "topology/xchg_shell.hpp"
#include "topology/xchg_face.hpp"
#include "topology/xchg_loop.hpp"
#include "topology/xchg_coedge.hpp"
#include "topology/xchg_edge.hpp"
#include "topology/xchg_vertex.hpp"

#include "geom/xchg_point.hpp"
#include "geom/geom.hpp"
#include "base/xchg_pnt.hpp"
#include "base/xchg_dir.hpp"
#include "geom/surface/xchg_planesurface.hpp"
#include "geom/surface/xchg_cylindricalsurface.hpp"

#include <cstdio>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// XchgTopoCompare — collect and compare Xchg_Body topology
//
// Usage:
//   // Collect
//   XchgBodyData ref  = XchgTopoCompare::Collect(refBody,  "STEP->Xchg");
//   XchgBodyData ours = XchgTopoCompare::Collect(oursBody, "PK->Xchg");
//
//   // Dump a single body (flat listing)
//   XchgTopoCompare::Dump(ref);
//
//   // Side-by-side diff table with <<<DIFF>>> markers
//   XchgTopoCompare::Compare(ref, ours);
// ---------------------------------------------------------------------------

// ---- Plain-old-data structures ----

struct XchgCEdgeData {
    int         inLoop;       // second param of AddCoedge (coedge/loop dir agreement)
    int         ceOrient;     // Xchg_Coedge::GetOrientation
    int         sameSense;    // Xchg_Edge::GetSameSense  (-1 if no edge)
    std::string startV;       // start vertex coords string
    std::string endV;         // end vertex coords string
    std::string curvType;     // 3D edge curve type name
    std::string uvcrvType;    // 2D UV pcurve type name ("-" if absent)
};

struct XchgLoopData {
    int outer;
    int orient;
    std::vector<XchgCEdgeData> coedges;
};

struct XchgFaceData {
    int         shellOrient;  // orientation flag from Shell::GetFace
    int         nLoops;
    std::string surfType;     // surface type name
    std::string surfParams;   // surface key parameters (origin, normal, radius…)
    std::vector<XchgLoopData> loops;
};

struct XchgShellData {
    int closed;
    int outer;
    std::vector<XchgFaceData> faces;
};

struct XchgLumpData {
    std::vector<XchgShellData> shells;
};

struct XchgBodyData {
    std::string label;
    std::vector<XchgLumpData> lumps;
};

// ---- Implementation ----

namespace XchgTopoCompare {

namespace detail {

inline std::string FmtPnt(const Xchg_pnt& p)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "(%.4f,%.4f,%.4f)", p.x(), p.y(), p.z());
    return buf;
}

inline std::string FmtDir(const Xchg_dir& d)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "[%.4f,%.4f,%.4f]", d.x(), d.y(), d.z());
    return buf;
}

inline std::string SurfaceTypeStr(XCHG_TYPE_ENUM t)
{
    switch (t) {
    case XCHG_TYPE_PLANE:                       return "Plane";
    case XCHG_TYPE_CYLINDRICAL_SURFACE:         return "Cylinder";
    case XCHG_TYPE_CONICAL_SURFACE:             return "Cone";
    case XCHG_TYPE_SPHERICAL_SURFACE:           return "Sphere";
    case XCHG_TYPE_TOROIDAL_SURFACE:            return "Torus";
    case XCHG_TYPE_SURFACE_OF_LINEAR_EXTRUSION: return "LinearExtrusion";
    case XCHG_TYPE_SURFACE_OF_REVOLUTION:       return "Revolution";
    case XCHG_TYPE_NURBS_SURFACE:               return "NURBS";
    case XCHG_TYPE_OFFSET_SURFACE:              return "Offset";
    default: { char buf[16]; snprintf(buf, sizeof(buf), "Surf(%d)", (int)t); return buf; }
    }
}

inline std::string CurveTypeStr(XCHG_TYPE_ENUM t)
{
    switch (t) {
    case XCHG_TYPE_LINE:            return "Line";
    case XCHG_TYPE_CIRCLE:          return "Circle";
    case XCHG_TYPE_ELLIPSE:         return "Ellipse";
    case XCHG_TYPE_NURBS_CURVE:     return "NURBS";
    case XCHG_TYPE_OFFSET_CURVE_3D: return "Offset3D";
    case XCHG_TYPE_OFFSET_CURVE_2D: return "Offset2D";
    default: { char buf[16]; snprintf(buf, sizeof(buf), "Crv(%d)", (int)t); return buf; }
    }
}

inline std::string SurfaceParams(const Xchg_SurfacePtr& surf)
{
    if (!surf) return "(null)";
    XCHG_TYPE_ENUM t = surf->GetType();
    std::string s = SurfaceTypeStr(t) + " ";
    if (t == XCHG_TYPE_PLANE) {
        auto* p = Xchg_PlaneSurface::DtkDynamicCast(surf.get());
        if (p) s += "O=" + FmtPnt(p->GetOrigin()) + " N=" + FmtDir(p->GetNormal());
    } else if (t == XCHG_TYPE_CYLINDRICAL_SURFACE) {
        auto* p = Xchg_CylindricalSurface::DtkDynamicCast(surf.get());
        if (p) {
            char buf[32]; snprintf(buf, sizeof(buf), "R=%.4f", p->GetRadius());
            s += "O=" + FmtPnt(p->GetOrigin()) + " Ax=" + FmtDir(p->GetZDirection()) + " " + buf;
        }
    } else {
        s += "(params omitted)";
    }
    return s;
}

inline std::string VertexCoord(const Xchg_VertexPtr& v)
{
    if (!v) return "(null)";
    Xchg_PointPtr pt = v->GetGeom();
    if (!pt) return "(no geom)";
    Xchg_Double64 x, y, z;
    pt->GetCoordinates(x, y, z);
    char buf[64]; snprintf(buf, sizeof(buf), "(%.4f,%.4f,%.4f)", x, y, z);
    return buf;
}

static const char* DIFF_MARK = "  <<<DIFF>>>";
static const char* OK_MARK   = "";

} // namespace detail

// Collect all topology/geometry data from a body into a plain-data snapshot.
inline XchgBodyData Collect(const Xchg_BodyPtr& body, const char* label)
{
    using namespace detail;
    XchgBodyData bd;
    bd.label = label;
    Xchg_Size_t nLumps = body->GetNumLumps();
    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        XchgLumpData ld;
        Xchg_Size_t nShells = lump->GetNumShells();
        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);
            XchgShellData sd;
            sd.closed = (int)shell->IsClosed();
            sd.outer  = (int)shell->IsOuter();
            Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
            for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                Xchg_FacePtr face;
                Xchg_bool faceOrient;
                shell->GetFace(fi, face, faceOrient);
                XchgFaceData fd;
                fd.shellOrient = (int)faceOrient;
                Xchg_SurfacePtr surf = face->GetGeom();
                if (surf) {
                    fd.surfType   = SurfaceTypeStr(surf->GetType());
                    fd.surfParams = SurfaceParams(surf);
                } else {
                    fd.surfType   = "(no surf)";
                    fd.surfParams = "(no surf)";
                }
                Xchg_Size_t nLoops = face->GetNumLoops();
                fd.nLoops = (int)nLoops;
                for (Xchg_Size_t lli = 0; lli < nLoops; ++lli) {
                    Xchg_LoopPtr loop;
                    face->GetLoop(lli, loop);
                    XchgLoopData lod;
                    lod.outer  = (int)loop->IsOuter();
                    lod.orient = (int)loop->GetOrientation();
                    Xchg_Size_t nCE = loop->GetNumCoedges();
                    for (Xchg_Size_t ci = 0; ci < nCE; ++ci) {
                        Xchg_CoedgePtr coedge;
                        Xchg_bool coedgeOrient;
                        loop->GetCoedge(ci, coedge, coedgeOrient);
                        XchgCEdgeData ced;
                        ced.inLoop    = (int)coedgeOrient;
                        ced.ceOrient  = (int)coedge->GetOrientation();
                        Xchg_CurvePtr uvcurve = coedge->GetGeom();
                        ced.uvcrvType = uvcurve ? CurveTypeStr(uvcurve->GetType()) : "-";
                        Xchg_EdgePtr edge;
                        coedge->GetEdge(edge);
                        if (edge) {
                            ced.sameSense = (int)edge->GetSameSense();
                            Xchg_VertexPtr sv, ev;
                            edge->GetStartVertex(sv);
                            edge->GetEndVertex(ev);
                            ced.startV   = VertexCoord(sv);
                            ced.endV     = VertexCoord(ev);
                            Xchg_CurvePtr crv = edge->GetGeom();
                            ced.curvType = crv ? CurveTypeStr(crv->GetType()) : "-";
                        } else {
                            ced.sameSense = -1;
                            ced.startV    = "(no edge)";
                            ced.endV      = "(no edge)";
                            ced.curvType  = "-";
                        }
                        lod.coedges.push_back(ced);
                    }
                    fd.loops.push_back(lod);
                }
                sd.faces.push_back(fd);
            }
            ld.shells.push_back(sd);
        }
        bd.lumps.push_back(ld);
    }
    return bd;
}

// Dump a single body's topology in flat listing form.
inline void Dump(const XchgBodyData& bd)
{
    printf("\n===== Xchg_Body Topo: %s =====\n", bd.label.c_str());
    printf("Lumps: %zu\n", bd.lumps.size());
    for (size_t li = 0; li < bd.lumps.size(); ++li) {
        const auto& ld = bd.lumps[li];
        printf("Lump[%zu]: shells=%zu\n", li, ld.shells.size());
        for (size_t si = 0; si < ld.shells.size(); ++si) {
            const auto& sd = ld.shells[si];
            printf("  Shell[%zu]: faces=%zu closed=%d outer=%d\n",
                   si, sd.faces.size(), sd.closed, sd.outer);
            for (size_t fi = 0; fi < sd.faces.size(); ++fi) {
                const auto& fd = sd.faces[fi];
                printf("    Face[%zu]: shellOrient=%d loops=%d surfType=%s\n",
                       fi, fd.shellOrient, fd.nLoops, fd.surfType.c_str());
                printf("      surfParams: %s\n", fd.surfParams.c_str());
                for (size_t lli = 0; lli < fd.loops.size(); ++lli) {
                    const auto& lod = fd.loops[lli];
                    printf("      Loop[%zu]: outer=%d orient=%d coedges=%zu\n",
                           lli, lod.outer, lod.orient, lod.coedges.size());
                    for (size_t ci = 0; ci < lod.coedges.size(); ++ci) {
                        const auto& ced = lod.coedges[ci];
                        printf("        CE[%zu]: inLoop=%d ceOrient=%d sameSense=%d "
                               "crv=%-8s uv=%-8s  %s -> %s\n",
                               ci, ced.inLoop, ced.ceOrient, ced.sameSense,
                               ced.curvType.c_str(), ced.uvcrvType.c_str(),
                               ced.startV.c_str(), ced.endV.c_str());
                    }
                }
            }
        }
    }
    printf("===== End: %s =====\n\n", bd.label.c_str());
}

// Print side-by-side diff table; differences are marked with <<<DIFF>>>.
inline void Compare(const XchgBodyData& ref, const XchgBodyData& ours)
{
    using namespace detail;

    printf("\n");
    printf("==============================================================================\n");
    printf("  TOPOLOGY COMPARISON: %-28s  vs  %s\n", ref.label.c_str(), ours.label.c_str());
    printf("==============================================================================\n");

    size_t nL = std::max(ref.lumps.size(), ours.lumps.size());
    printf("  Lumps: REF=%-3zu  OURS=%-3zu%s\n",
           ref.lumps.size(), ours.lumps.size(),
           ref.lumps.size() == ours.lumps.size() ? OK_MARK : DIFF_MARK);

    for (size_t li = 0; li < nL; ++li) {
        const XchgLumpData* lr = (li < ref.lumps.size())  ? &ref.lumps[li]  : nullptr;
        const XchgLumpData* lo = (li < ours.lumps.size()) ? &ours.lumps[li] : nullptr;
        printf("  Lump[%zu]\n", li);

        size_t rs = lr ? lr->shells.size() : 0;
        size_t os = lo ? lo->shells.size() : 0;
        printf("    shells: REF=%-3zu  OURS=%-3zu%s\n", rs, os, rs == os ? OK_MARK : DIFF_MARK);

        size_t nS = std::max(rs, os);
        for (size_t si = 0; si < nS; ++si) {
            const XchgShellData* sr = (lr && si < lr->shells.size()) ? &lr->shells[si] : nullptr;
            const XchgShellData* so = (lo && si < lo->shells.size()) ? &lo->shells[si] : nullptr;

            int rc_c = sr ? sr->closed : -1, oc_c = so ? so->closed : -1;
            int rc_o = sr ? sr->outer  : -1, oc_o = so ? so->outer  : -1;
            size_t rf_n = sr ? sr->faces.size() : 0;
            size_t of_n = so ? so->faces.size() : 0;

            printf("    Shell[%zu]: closed REF=%d OURS=%d%s  outer REF=%d OURS=%d%s  faces REF=%zu OURS=%zu%s\n",
                   si,
                   rc_c, oc_c, rc_c == oc_c ? OK_MARK : DIFF_MARK,
                   rc_o, oc_o, rc_o == oc_o ? OK_MARK : DIFF_MARK,
                   rf_n, of_n, rf_n == of_n ? OK_MARK : DIFF_MARK);

            size_t nF = std::max(rf_n, of_n);
            for (size_t fi = 0; fi < nF; ++fi) {
                const XchgFaceData* fr = (sr && fi < sr->faces.size()) ? &sr->faces[fi] : nullptr;
                const XchgFaceData* fo = (so && fi < so->faces.size()) ? &so->faces[fi] : nullptr;

                int rso = fr ? fr->shellOrient : -1, oso = fo ? fo->shellOrient : -1;
                int rlo = fr ? fr->nLoops      : -1, olo = fo ? fo->nLoops      : -1;
                std::string rST = fr ? fr->surfType : "(missing)";
                std::string oST = fo ? fo->surfType : "(missing)";

                printf("      Face[%zu]: shellOrient REF=%d OURS=%d%s  loops REF=%d OURS=%d%s  surfType REF=%s OURS=%s%s\n",
                       fi,
                       rso, oso, rso == oso ? OK_MARK : DIFF_MARK,
                       rlo, olo, rlo == olo ? OK_MARK : DIFF_MARK,
                       rST.c_str(), oST.c_str(), rST == oST ? OK_MARK : DIFF_MARK);

                if (fr && fo && fr->surfParams != fo->surfParams) {
                    printf("        surfParams:\n");
                    printf("          REF : %s\n", fr->surfParams.c_str());
                    printf("          OURS: %s%s\n", fo->surfParams.c_str(), DIFF_MARK);
                }

                size_t nLL = std::max(fr ? fr->loops.size() : (size_t)0,
                                      fo ? fo->loops.size() : (size_t)0);
                for (size_t lli = 0; lli < nLL; ++lli) {
                    const XchgLoopData* lr2 = (fr && lli < fr->loops.size()) ? &fr->loops[lli] : nullptr;
                    const XchgLoopData* lo2 = (fo && lli < fo->loops.size()) ? &fo->loops[lli] : nullptr;

                    int rot = lr2 ? lr2->outer  : -1, oot = lo2 ? lo2->outer  : -1;
                    int ror = lr2 ? lr2->orient : -1, oor = lo2 ? lo2->orient : -1;
                    size_t rce = lr2 ? lr2->coedges.size() : 0;
                    size_t oce = lo2 ? lo2->coedges.size() : 0;

                    printf("        Loop[%zu]: outer REF=%d OURS=%d%s  orient REF=%d OURS=%d%s  coedges REF=%zu OURS=%zu%s\n",
                           lli,
                           rot, oot, rot == oot ? OK_MARK : DIFF_MARK,
                           ror, oor, ror == oor ? OK_MARK : DIFF_MARK,
                           rce, oce, rce == oce ? OK_MARK : DIFF_MARK);

                    size_t nCE = std::max(rce, oce);
                    for (size_t ci = 0; ci < nCE; ++ci) {
                        const XchgCEdgeData* cr = (lr2 && ci < lr2->coedges.size()) ? &lr2->coedges[ci] : nullptr;
                        const XchgCEdgeData* co = (lo2 && ci < lo2->coedges.size()) ? &lo2->coedges[ci] : nullptr;

                        int ril  = cr ? cr->inLoop    : -1, oil  = co ? co->inLoop    : -1;
                        int rce2 = cr ? cr->ceOrient  : -1, oce2 = co ? co->ceOrient  : -1;
                        int rss  = cr ? cr->sameSense : -1, oss  = co ? co->sameSense : -1;
                        std::string rSV = cr ? cr->startV    : "(miss)";
                        std::string oSV = co ? co->startV    : "(miss)";
                        std::string rEV = cr ? cr->endV      : "(miss)";
                        std::string oEV = co ? co->endV      : "(miss)";
                        std::string rCT = cr ? cr->curvType  : "(miss)";
                        std::string oCT = co ? co->curvType  : "(miss)";
                        std::string rUV = cr ? cr->uvcrvType : "(miss)";
                        std::string oUV = co ? co->uvcrvType : "(miss)";

                        printf("          CE[%zu]:\n", ci);
                        printf("            inLoop    : REF=%d  OURS=%d%s\n",   ril,  oil,  ril  == oil  ? OK_MARK : DIFF_MARK);
                        printf("            ceOrient  : REF=%d  OURS=%d%s\n",   rce2, oce2, rce2 == oce2 ? OK_MARK : DIFF_MARK);
                        printf("            sameSense : REF=%d  OURS=%d%s\n",   rss,  oss,  rss  == oss  ? OK_MARK : DIFF_MARK);
                        printf("            curveType : REF=%-12s  OURS=%-12s%s\n", rCT.c_str(), oCT.c_str(), rCT == oCT ? OK_MARK : DIFF_MARK);
                        printf("            uvCrvType : REF=%-12s  OURS=%-12s%s\n", rUV.c_str(), oUV.c_str(), rUV == oUV ? OK_MARK : DIFF_MARK);
                        printf("            startV    : REF=%-28s  OURS=%-28s%s\n", rSV.c_str(), oSV.c_str(), rSV == oSV ? OK_MARK : DIFF_MARK);
                        printf("            endV      : REF=%-28s  OURS=%-28s%s\n", rEV.c_str(), oEV.c_str(), rEV == oEV ? OK_MARK : DIFF_MARK);
                    }
                }
            }
        }
    }
    printf("==============================================================================\n\n");
}

// Convenience overload: collect + compare in one call.
inline void Compare(const Xchg_BodyPtr& refBody,  const char* refLabel,
                    const Xchg_BodyPtr& oursBody, const char* oursLabel)
{
    Compare(Collect(refBody, refLabel), Collect(oursBody, oursLabel));
}

} // namespace XchgTopoCompare
