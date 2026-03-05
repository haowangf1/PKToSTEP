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


// Dump a single body's topology in a human-friendly hierarchical format.
inline void DumpBody(const Xchg_BodyPtr& body, const char* label)
{
    using namespace detail;

    printf("\n");
    printf("================================================================================\n");
    printf("  Xchg_Body Structure: %s\n", label);
    printf("================================================================================\n");

    // Count topology entities
    int totalLumps = 0, totalShells = 0, totalFaces = 0;
    int totalLoops = 0, totalCoedges = 0;

    Xchg_Size_t nLumps = body->GetNumLumps();
    totalLumps = (int)nLumps;

    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        Xchg_Size_t nShells = lump->GetNumShells();
        totalShells += (int)nShells;

        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);
            Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
            totalFaces += (int)nFaces;

            for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                Xchg_FacePtr face;
                Xchg_bool faceOrient;
                shell->GetFace(fi, face, faceOrient);
                Xchg_Size_t nLoops = face->GetNumLoops();
                totalLoops += (int)nLoops;

                for (Xchg_Size_t lli = 0; lli < nLoops; ++lli) {
                    Xchg_LoopPtr loop;
                    face->GetLoop(lli, loop);
                    Xchg_Size_t nCE = loop->GetNumCoedges();
                    totalCoedges += (int)nCE;
                }
            }
        }
    }

    // Print topology statistics
    printf("\n[Topology Statistics]\n");
    printf("  Lumps    : %d\n", totalLumps);
    printf("  Shells   : %d\n", totalShells);
    printf("  Faces    : %d\n", totalFaces);
    printf("  Loops    : %d\n", totalLoops);
    printf("  Coedges  : %d\n", totalCoedges);

    // Print hierarchical structure
    printf("\n[Hierarchical Structure]\n");

    for (Xchg_Size_t li = 0; li < nLumps; ++li) {
        Xchg_LumpPtr lump;
        body->GetLump(li, lump);
        printf("\n  Lump[%zu]\n", li);

        Xchg_Size_t nShells = lump->GetNumShells();
        for (Xchg_Size_t si = 0; si < nShells; ++si) {
            Xchg_ShellPtr shell;
            lump->GetShell(si, shell);

            printf("    |\n");
            printf("    +-- Shell[%zu]  [closed=%d, outer=%d]\n",
                   si, (int)shell->IsClosed(), (int)shell->IsOuter());

            Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
            for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                Xchg_FacePtr face;
                Xchg_bool faceOrient;
                shell->GetFace(fi, face, faceOrient);

                Xchg_SurfacePtr surf = face->GetGeom();
                std::string surfInfo = surf ? SurfaceTypeStr(surf->GetType()) : "(no surf)";

                printf("    |   |\n");
                printf("    |   +-- Face[%zu]  [shellOrient=%d, surf=%s]\n",
                       fi, (int)faceOrient, surfInfo.c_str());

                // Print geometry parameters
                if (surf) {
                    printf("    |   |      Geom: %s\n", SurfaceParams(surf).c_str());
                }

                Xchg_Size_t nLoops = face->GetNumLoops();
                for (Xchg_Size_t lli = 0; lli < nLoops; ++lli) {
                    Xchg_LoopPtr loop;
                    face->GetLoop(lli, loop);

                    printf("    |   |   |\n");
                    printf("    |   |   +-- Loop[%zu]  [outer=%d, orient=%d, coedges=%zu]\n",
                           lli, (int)loop->IsOuter(), (int)loop->GetOrientation(),
                           loop->GetNumCoedges());

                    Xchg_Size_t nCE = loop->GetNumCoedges();
                    for (Xchg_Size_t ci = 0; ci < nCE; ++ci) {
                        Xchg_CoedgePtr coedge;
                        Xchg_bool coedgeOrient;
                        loop->GetCoedge(ci, coedge, coedgeOrient);

                        Xchg_EdgePtr edge;
                        coedge->GetEdge(edge);

                        std::string edgeInfo = "(no edge)";
                        if (edge) {
                            Xchg_VertexPtr sv, ev;
                            edge->GetStartVertex(sv);
                            edge->GetEndVertex(ev);
                            Xchg_CurvePtr crv = edge->GetGeom();
                            std::string crvType = crv ? CurveTypeStr(crv->GetType()) : "-";
                            edgeInfo = crvType + " " + VertexCoord(sv) + " -> " + VertexCoord(ev);
                        }

                        printf("    |   |   |   |\n");
                        printf("    |   |   |   +-- Coedge[%zu]  [inLoop=%d, orient=%d]\n",
                               ci, (int)coedgeOrient, (int)coedge->GetOrientation());
                        printf("    |   |   |   |      Edge: %s\n", edgeInfo.c_str());
                    }
                }
            }
        }
    }

    printf("\n================================================================================\n\n");
}


} // namespace XchgTopoCompare
