#include "pk_to_xchg_converter.hpp"

#include "step/translator_step.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_maindoc.hpp"
#include "xchg_component.hpp"
#include "xchg_node.hpp"
#include "util/xchg_kernel_utils.hpp"
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

#include <cstdio>
#include <string>
#include <set>
#include <map>
#include <vector>
#include <cmath>
#include <sstream>

// ---------------------------------------------------------------------------
// Topology comparison helpers
// ---------------------------------------------------------------------------

static std::string FmtPnt(const Xchg_pnt& p)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "(%.4f,%.4f,%.4f)", p.x(), p.y(), p.z());
    return buf;
}

static std::string FmtDir(const Xchg_dir& d)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "[%.4f,%.4f,%.4f]", d.x(), d.y(), d.z());
    return buf;
}

static std::string SurfaceTypeStr(XCHG_TYPE_ENUM t)
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

static std::string CurveTypeStr(XCHG_TYPE_ENUM t)
{
    switch (t) {
    case XCHG_TYPE_LINE:           return "Line";
    case XCHG_TYPE_CIRCLE:         return "Circle";
    case XCHG_TYPE_ELLIPSE:        return "Ellipse";
    case XCHG_TYPE_NURBS_CURVE:    return "NURBS";
    case XCHG_TYPE_OFFSET_CURVE_3D:return "Offset3D";
    case XCHG_TYPE_OFFSET_CURVE_2D:return "Offset2D";
    default: { char buf[16]; snprintf(buf, sizeof(buf), "Crv(%d)", (int)t); return buf; }
    }
}

// Surface key parameters as a short string for diff
static std::string SurfaceParams(const Xchg_SurfacePtr& surf)
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

// Vertex coordinates as string
static std::string VertexCoord(const Xchg_VertexPtr& v)
{
    if (!v) return "(null)";
    Xchg_PointPtr pt = v->GetGeom();
    if (!pt) return "(no geom)";
    Xchg_Double64 x, y, z;
    pt->GetCoordinates(x, y, z);
    char buf[64]; snprintf(buf, sizeof(buf), "(%.4f,%.4f,%.4f)", x, y, z);
    return buf;
}

// ---------------------------------------------------------------------------
// Data structures for collected topology
// ---------------------------------------------------------------------------
struct CEdgeData {
    int    inLoop;      // coedge->loop orientation param
    int    ceOrient;    // coedge->GetOrientation
    int    sameSense;   // edge->GetSameSense
    std::string startV;
    std::string endV;
    std::string curvType; // 3D curve type on edge
    std::string uvcrvType;// 2D UV curve on coedge
};
struct LoopData {
    int outer;
    int orient;
    std::vector<CEdgeData> coedges;
};
struct FaceData {
    int shellOrient;
    int nLoops;
    std::string surfType;
    std::string surfParams;
    std::vector<LoopData> loops;
};
struct ShellData {
    int closed;
    int outer;
    std::vector<FaceData> faces;
};
struct LumpData {
    std::vector<ShellData> shells;
};
struct BodyData {
    std::string label;
    std::vector<LumpData> lumps;
};

static BodyData CollectBodyData(const Xchg_BodyPtr& body, const char* label)
{
    BodyData bd;
    bd.label = label;
    Xchg_Size_t nLumps = body->GetNumLumps();
    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        LumpData ld;
        Xchg_Size_t nShells = lump->GetNumShells();
        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);
            ShellData sd;
            sd.closed = (int)shell->IsClosed();
            sd.outer  = (int)shell->IsOuter();
            Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
            for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                Xchg_FacePtr face;
                Xchg_bool faceOrient;
                shell->GetFace(fi, face, faceOrient);
                FaceData fd;
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
                    LoopData lod;
                    lod.outer  = (int)loop->IsOuter();
                    lod.orient = (int)loop->GetOrientation();
                    Xchg_Size_t nCE = loop->GetNumCoedges();
                    for (Xchg_Size_t ci = 0; ci < nCE; ++ci) {
                        Xchg_CoedgePtr coedge;
                        Xchg_bool coedgeOrient;
                        loop->GetCoedge(ci, coedge, coedgeOrient);
                        CEdgeData ced;
                        ced.inLoop   = (int)coedgeOrient;
                        ced.ceOrient = (int)coedge->GetOrientation();
                        Xchg_CurvePtr uvcurve = coedge->GetGeom();
                        ced.uvcrvType = uvcurve ? CurveTypeStr(uvcurve->GetType()) : "-";
                        Xchg_EdgePtr edge;
                        coedge->GetEdge(edge);
                        if (edge) {
                            ced.sameSense = (int)edge->GetSameSense();
                            Xchg_VertexPtr sv, ev;
                            edge->GetStartVertex(sv);
                            edge->GetEndVertex(ev);
                            ced.startV = VertexCoord(sv);
                            ced.endV   = VertexCoord(ev);
                            Xchg_CurvePtr crv = edge->GetGeom();
                            ced.curvType = crv ? CurveTypeStr(crv->GetType()) : "-";
                        } else {
                            ced.sameSense = -1;
                            ced.startV = "(no edge)";
                            ced.endV   = "(no edge)";
                            ced.curvType = "-";
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

// ---------------------------------------------------------------------------
// Table-based comparison output
// ---------------------------------------------------------------------------

static const char* DIFF_MARK = "  <<<DIFF>>>";
static const char* OK_MARK   = "";

static std::string CmpStr(const std::string& a, const std::string& b)
{
    return (a == b) ? OK_MARK : DIFF_MARK;
}

static void CompareXchgBodyTopo(const Xchg_BodyPtr& refBody, const Xchg_BodyPtr& oursBody)
{
    BodyData ref  = CollectBodyData(refBody,  "STEP->Xchg (reference)");
    BodyData ours = CollectBodyData(oursBody, "PK->Xchg (ours)");

    printf("\n");
    printf("==============================================================================\n");
    printf("  TOPOLOGY COMPARISON: %-28s  vs  %s\n", ref.label.c_str(), ours.label.c_str());
    printf("==============================================================================\n");

    // Lump count
    size_t nL = std::max(ref.lumps.size(), ours.lumps.size());
    {
        const char* diff = (ref.lumps.size() == ours.lumps.size()) ? OK_MARK : DIFF_MARK;
        printf("  Lumps: REF=%-3zu  OURS=%-3zu%s\n", ref.lumps.size(), ours.lumps.size(), diff);
    }

    for (size_t li = 0; li < nL; ++li) {
        const LumpData* lr = (li < ref.lumps.size())  ? &ref.lumps[li]  : nullptr;
        const LumpData* lo = (li < ours.lumps.size()) ? &ours.lumps[li] : nullptr;
        printf("  Lump[%zu]\n", li);

        size_t nS = std::max(lr ? lr->shells.size() : 0, lo ? lo->shells.size() : 0);
        {
            size_t rs = lr ? lr->shells.size() : 0;
            size_t os = lo ? lo->shells.size() : 0;
            printf("    shells: REF=%-3zu  OURS=%-3zu%s\n", rs, os,
                   rs == os ? OK_MARK : DIFF_MARK);
        }

        for (size_t si = 0; si < nS; ++si) {
            const ShellData* sr = (lr && si < lr->shells.size()) ? &lr->shells[si] : nullptr;
            const ShellData* so = (lo && si < lo->shells.size()) ? &lo->shells[si] : nullptr;

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
                const FaceData* fr = (sr && fi < sr->faces.size()) ? &sr->faces[fi] : nullptr;
                const FaceData* fo = (so && fi < so->faces.size()) ? &so->faces[fi] : nullptr;

                int rso = fr ? fr->shellOrient : -1, oso = fo ? fo->shellOrient : -1;
                int rlo = fr ? fr->nLoops : -1,      olo = fo ? fo->nLoops : -1;
                std::string rST = fr ? fr->surfType : "(missing)";
                std::string oST = fo ? fo->surfType : "(missing)";

                printf("      Face[%zu]: shellOrient REF=%d OURS=%d%s  loops REF=%d OURS=%d%s  surfType REF=%s OURS=%s%s\n",
                       fi,
                       rso, oso, rso == oso ? OK_MARK : DIFF_MARK,
                       rlo, olo, rlo == olo ? OK_MARK : DIFF_MARK,
                       rST.c_str(), oST.c_str(), rST == oST ? OK_MARK : DIFF_MARK);

                // Surface params detail
                if (fr && fo) {
                    bool paramSame = (fr->surfParams == fo->surfParams);
                    if (!paramSame) {
                        printf("        surfParams:\n");
                        printf("          REF : %s\n", fr->surfParams.c_str());
                        printf("          OURS: %s%s\n", fo->surfParams.c_str(), DIFF_MARK);
                    }
                }

                size_t nLL = std::max(fr ? fr->loops.size() : (size_t)0,
                                      fo ? fo->loops.size() : (size_t)0);
                for (size_t lli = 0; lli < nLL; ++lli) {
                    const LoopData* lr2 = (fr && lli < fr->loops.size()) ? &fr->loops[lli] : nullptr;
                    const LoopData* lo2 = (fo && lli < fo->loops.size()) ? &fo->loops[lli] : nullptr;

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
                        const CEdgeData* cr = (lr2 && ci < lr2->coedges.size()) ? &lr2->coedges[ci] : nullptr;
                        const CEdgeData* co = (lo2 && ci < lo2->coedges.size()) ? &lo2->coedges[ci] : nullptr;

                        int ril = cr ? cr->inLoop   : -1, oil = co ? co->inLoop   : -1;
                        int rce2= cr ? cr->ceOrient : -1, oce2= co ? co->ceOrient : -1;
                        int rss = cr ? cr->sameSense: -1, oss = co ? co->sameSense: -1;
                        std::string rSV = cr ? cr->startV    : "(miss)";
                        std::string oSV = co ? co->startV    : "(miss)";
                        std::string rEV = cr ? cr->endV      : "(miss)";
                        std::string oEV = co ? co->endV      : "(miss)";
                        std::string rCT = cr ? cr->curvType  : "(miss)";
                        std::string oCT = co ? co->curvType  : "(miss)";
                        std::string rUV = cr ? cr->uvcrvType : "(miss)";
                        std::string oUV = co ? co->uvcrvType : "(miss)";

                        printf("          CE[%zu]:\n", ci);
                        printf("            inLoop    : REF=%d  OURS=%d%s\n", ril, oil, ril==oil?OK_MARK:DIFF_MARK);
                        printf("            ceOrient  : REF=%d  OURS=%d%s\n", rce2, oce2, rce2==oce2?OK_MARK:DIFF_MARK);
                        printf("            sameSense : REF=%d  OURS=%d%s\n", rss, oss, rss==oss?OK_MARK:DIFF_MARK);
                        printf("            curveType : REF=%-12s  OURS=%-12s%s\n", rCT.c_str(), oCT.c_str(), rCT==oCT?OK_MARK:DIFF_MARK);
                        printf("            uvCrvType : REF=%-12s  OURS=%-12s%s\n", rUV.c_str(), oUV.c_str(), rUV==oUV?OK_MARK:DIFF_MARK);
                        printf("            startV    : REF=%-28s  OURS=%-28s%s\n", rSV.c_str(), oSV.c_str(), rSV==oSV?OK_MARK:DIFF_MARK);
                        printf("            endV      : REF=%-28s  OURS=%-28s%s\n", rEV.c_str(), oEV.c_str(), rEV==oEV?OK_MARK:DIFF_MARK);
                    }
                }
            }
        }
    }
    printf("==============================================================================\n\n");
}

static bool TransmitBodyToXT(PK_BODY_t body, const char* output_path)
{
    PK_PART_transmit_o_t transmit_opts;
    PK_PART_transmit_o_m(transmit_opts);
    transmit_opts.transmit_format = PK_transmit_format_text_c;

    PK_BODY_t parts[] = { body };
    PK_ERROR_code_t err = PK_PART_transmit(1, parts, output_path, &transmit_opts);
    if (err != PK_ERROR_no_errors) {
        fprintf(stderr, "[Error] PK_PART_transmit failed: %d\n", err);
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    // 以 exe 所在目录为基准，向上找到项目根目录（exe 在 build/Debug/Debug/ 下，上三级是根目录）
    std::string exe_dir = argv[0];
    for (char& c : exe_dir) if (c == '\\') c = '/';
    auto last_slash = exe_dir.rfind('/');
    std::string base = (last_slash != std::string::npos) ? exe_dir.substr(0, last_slash + 1) : "./";
    // build/Debug/Debug/ -> ../../.. 即项目根目录
    base += "../../../";

    std::string step_path = base + "resource/cube214.step";

    // 从 step 文件路径提取不带扩展名的文件名，拼到同目录下加 _roundtrip 后缀
    // PK_PART_transmit 导出时会自动追加 .xmt_txt
    auto name_start = step_path.rfind('/');
    std::string stem = (name_start != std::string::npos)
                       ? step_path.substr(name_start + 1) : step_path;
    auto ext_pos = stem.rfind('.');
    if (ext_pos != std::string::npos)
        stem = stem.substr(0, ext_pos);
    std::string xt_path = base + "xt/" + stem + "_roundtrip";

    // 1. Start Parasolid session
    Xchg_Int32 pk_err = StartSession();
    if (pk_err != 0) {
        fprintf(stderr, "[Error] StartSession failed: %d\n", pk_err);
        return 1;
    }
    printf("[Info] Parasolid session started.\n");

    // 2. Read STEP file -> Xchg_MainDoc
    AMXT_STP_CTX_context_p ctx = nullptr;
    AMXT_STP_ERROR_code_t stp_err = AMXT_STP_CTX_create(&ctx);
    if (stp_err != AMXT_STP_ERROR_no_errors) {
        fprintf(stderr, "[Error] AMXT_STP_CTX_create failed: %d\n", stp_err);
        StopSession();
        return 1;
    }

    AMXT_STP_read_o_t read_opts;
    AMXT_STP_read_o_m(read_opts);

    Xchg_MainDocPtr main_doc = Xchg_MainDoc::Create();
    stp_err = AMXT_STP_read(ctx, step_path.c_str(), &read_opts, &main_doc);
    if (stp_err != AMXT_STP_ERROR_no_errors) {
        size_t err_instance = 0;
        const char* err_msg = nullptr;
        AMXT_STP_ERROR_ask_message(ctx, &err_instance, &err_msg);
        fprintf(stderr, "[Error] AMXT_STP_read failed: %d, entity #%zu: %s\n",
                stp_err, err_instance, err_msg ? err_msg : "unknown");
        AMXT_STP_CTX_destory(ctx);
        StopSession();
        return 1;
    }
    printf("[Info] STEP file read successfully: %s\n", step_path.c_str());

    // 3. Get root component, iterate nodes to find Body nodes
    const Xchg_ComponentPtr& root = main_doc->RootComponent();
    if (!root) {
        fprintf(stderr, "[Error] MainDoc has no root component.\n");
        AMXT_STP_CTX_destory(ctx);
        StopSession();
        return 1;
    }

    Xchg_Size_t num_nodes = root->GetNumNodes();
    printf("[Info] Root component has %zu nodes.\n", num_nodes);

    int body_count = 0;
    for (Xchg_Size_t i = 0; i < num_nodes; ++i) {
        Xchg_NodePtr node = root->GetNodeByIndex(i);
        if (!node || node->GetNodeType() != Xchg_Node::BodyType)
            continue;

        Xchg_BodyPtr xchg_body = node->GetBodyPtr();
        if (!xchg_body) {
            fprintf(stderr, "[Warn] Node %zu is BodyType but GetBodyPtr() returned null.\n", i);
            continue;
        }

        printf("[Info] Found Body node at index %zu.\n", i);

        // 4. Convert Xchg_Body -> PK_BODY via Xchg_Node::ConvertToPKBody
        Xchg_Int32 convert_err = node->ConvertToPKBody();
        if (convert_err != 0) {
            fprintf(stderr, "[Error] ConvertToPKBody failed: %d\n", convert_err);
            continue;
        }
        Xchg_Int32 pk_body_tag = node->GetParasolidBody();
        if (pk_body_tag == 0) {
            fprintf(stderr, "[Error] GetParasolidBody returned null.\n");
            continue;
        }
        printf("[Info] Converted to PK_BODY tag: %d\n", pk_body_tag);

        // 5. PK_BODY -> Xchg_Body (our converter) -> PK_BODY -> .x_t roundtrip
        PK_BODY_t original_pk_body = static_cast<PK_BODY_t>(pk_body_tag);

        PKToXchgConverter converter;
        converter.SetLogCallback([](const std::string& msg) {
            fprintf(stderr, "[Converter] %s\n", msg.c_str());
        });

        Xchg_BodyPtr roundtrip_xchg_body;
        STEPExport_ErrorCode rc = converter.Convert(original_pk_body, &roundtrip_xchg_body);
        if (rc != STEP_OK || !roundtrip_xchg_body) {
            fprintf(stderr, "[Error] PKToXchgConverter::Convert failed: %d\n", rc);
            continue;
        }
        printf("[Info] PK_BODY -> Xchg_Body succeeded.\n");
        CompareXchgBodyTopo(xchg_body, roundtrip_xchg_body);

        Xchg_MainDocPtr rt_doc = Xchg_MainDoc::Create();
        Xchg_ComponentPtr rt_comp = rt_doc->CreateComponent(
            L"roundtrip", L"roundtrip", Xchg_Component::ComponentInternal);
        rt_doc->SetRootComponent(rt_comp);
        Xchg_NodePtr rt_node = rt_comp->CreateBodyNode(L"body", roundtrip_xchg_body, 0);

        Xchg_Int32 rt_err = rt_node->ConvertToPKBody();
        if (rt_err != 0) {
            fprintf(stderr, "[Error] Roundtrip ConvertToPKBody failed: %d\n", rt_err);
            continue;
        }
        Xchg_Int32 rt_pk_body = rt_node->GetParasolidBody();
        if (rt_pk_body == 0) {
            fprintf(stderr, "[Error] Roundtrip GetParasolidBody returned null.\n");
            continue;
        }
        printf("[Info] Xchg_Body -> PK_BODY roundtrip succeeded, new tag: %d\n", rt_pk_body);

        // 6. Transmit roundtrip PK_BODY to .x_t file
        std::string out = xt_path;
        if (body_count > 0) {
            out = xt_path + "." + std::to_string(body_count);
        }

        if (TransmitBodyToXT(static_cast<PK_BODY_t>(rt_pk_body), out.c_str())) {
            printf("[Info] Written to: %s\n", out.c_str());
        }
        ++body_count;
    }

    if (body_count == 0) {
        fprintf(stderr, "[Warn] No body nodes found in root component.\n");
    } else {
        printf("[Info] Total %d bodies converted.\n", body_count);
    }

    // 7. Cleanup
    AMXT_STP_CTX_destory(ctx);
    StopSession();
    printf("[Info] Done.\n");
    return 0;
}
