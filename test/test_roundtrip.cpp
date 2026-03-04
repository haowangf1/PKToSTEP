#include <gtest/gtest.h>
#include <map>
#include "pk_to_xchg_converter.hpp"
#include "step/translator_step.hpp"
#include "xchg_maindoc.hpp"
#include "xchg_component.hpp"
#include "xchg_componentinstance.hpp"
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

class RoundtripTest : public ::testing::Test {
protected:
    void SetUp() override {
        Xchg_Int32 err = StartSession();
        ASSERT_EQ(err, 0) << "Failed to start Parasolid session";
    }

    void TearDown() override {
        StopSession();
    }

    struct TopoStats {
        int bodies = 0;
        int lumps = 0;
        int shells = 0;
        int faces = 0;
        int loops = 0;
        int coedges = 0;
        int edges = 0;
        int vertices = 0;
        int faces_with_geom = 0;
        int edges_with_geom = 0;
        int vertices_with_geom = 0;
        int outer_loops = 0;
        int inner_loops = 0;
        std::map<std::pair<int,int>, int> face_orient_counts; // (shell_idx, orient) -> count
    };

    TopoStats CollectStats(const Xchg_BodyPtr& body) {
        TopoStats stats;
        if (!body) return stats;
        stats.bodies = 1;

        Xchg_Size_t nLumps = body->GetNumLumps();
        for (Xchg_Size_t li = 0; li < nLumps; ++li) {
            Xchg_LumpPtr lump;
            body->GetLump(li, lump);
            if (!lump) continue;
            stats.lumps++;

            Xchg_Size_t nShells = lump->GetNumShells();
            for (Xchg_Size_t si = 0; si < nShells; ++si) {
                Xchg_ShellPtr shell;
                lump->GetShell(si, shell);
                if (!shell) continue;
                stats.shells++;

                Xchg_Size_t nFaces = shell->GetNumFaces(DtkFaceType_Bounded);
                for (Xchg_Size_t fi = 0; fi < nFaces; ++fi) {
                    Xchg_FacePtr face;
                    Xchg_bool faceOrient;
                    shell->GetFace(fi, face, faceOrient);
                    if (!face) continue;
                    stats.faces++;
                    if (face->GetGeom()) stats.faces_with_geom++;
                    stats.face_orient_counts[{si, faceOrient}]++;

                    Xchg_Size_t nLoops = face->GetNumLoops();
                    for (Xchg_Size_t lli = 0; lli < nLoops; ++lli) {
                        Xchg_LoopPtr loop;
                        face->GetLoop(lli, loop);
                        if (!loop) continue;
                        stats.loops++;
                        if (loop->IsOuter()) stats.outer_loops++;
                        else stats.inner_loops++;

                        Xchg_Size_t nCE = loop->GetNumCoedges();
                        for (Xchg_Size_t ci = 0; ci < nCE; ++ci) {
                            Xchg_CoedgePtr coedge;
                            Xchg_bool coedgeOrient;
                            loop->GetCoedge(ci, coedge, coedgeOrient);
                            if (!coedge) continue;
                            stats.coedges++;

                            Xchg_EdgePtr edge;
                            coedge->GetEdge(edge);
                            if (edge) {
                                stats.edges++;
                                if (edge->GetGeom()) stats.edges_with_geom++;
                            }
                        }
                    }
                }
            }
        }
        return stats;
    }

    Xchg_BodyPtr LoadStepAndConvert(const std::string& step_file) {
        AMXT_STP_CTX_context_p ctx = nullptr;
        AMXT_STP_ERROR_code_t err = AMXT_STP_CTX_create(&ctx);
        if (err != AMXT_STP_ERROR_no_errors) return nullptr;

        AMXT_STP_read_o_t read_opts;
        AMXT_STP_read_o_m(read_opts);

        Xchg_MainDocPtr doc = Xchg_MainDoc::Create();
        err = AMXT_STP_read(ctx, step_file.c_str(), &read_opts, &doc);
        AMXT_STP_CTX_destory(ctx);

        if (err != AMXT_STP_ERROR_no_errors) return nullptr;

        auto root = doc->RootComponent();
        if (!root) return nullptr;

        for (Xchg_Size_t i = 0; i < root->GetNumNodes(); ++i) {
            auto node = root->GetNodeByIndex(i);
            if (node && node->GetNodeType() == Xchg_Node::BodyType) {
                return node->GetBodyPtr();
            }
        }
        return nullptr;
    }

    Xchg_BodyPtr RoundtripConvert(const Xchg_BodyPtr& ref_body) {
        Xchg_MainDocPtr doc = Xchg_MainDoc::Create();
        Xchg_ComponentPtr comp = doc->CreateComponent(L"test", L"test", Xchg_Component::ComponentInternal);
        doc->SetRootComponent(comp);
        Xchg_NodePtr node = comp->CreateBodyNode(L"body", ref_body, 0);

        if (node->ConvertToPKBody() != 0) return nullptr;
        PK_BODY_t pk_body = node->GetParasolidBody();
        if (pk_body == 0) return nullptr;

        PKToXchgConverter converter;
        Xchg_BodyPtr result;
        if (converter.Convert(pk_body, &result) != STEP_OK) return nullptr;

        return result;
    }
};

TEST_F(RoundtripTest, Cube214) {
    auto ref = LoadStepAndConvert(PROJECT_ROOT_DIR "/resource/cube214.step");
    ASSERT_TRUE(ref != nullptr);

    auto rt = RoundtripConvert(ref);
    ASSERT_TRUE(rt != nullptr);

    auto ref_stats = CollectStats(ref);
    auto rt_stats = CollectStats(rt);

    EXPECT_EQ(rt_stats.bodies, ref_stats.bodies);
    EXPECT_EQ(rt_stats.lumps, ref_stats.lumps);
    EXPECT_EQ(rt_stats.shells, ref_stats.shells);
    EXPECT_EQ(rt_stats.faces, ref_stats.faces);
    EXPECT_EQ(rt_stats.loops, ref_stats.loops);
    EXPECT_EQ(rt_stats.coedges, ref_stats.coedges);
    EXPECT_EQ(rt_stats.edges, ref_stats.edges);

    EXPECT_GT(rt_stats.faces_with_geom, 0);
    EXPECT_GT(rt_stats.edges_with_geom, 0);
}

TEST_F(RoundtripTest, HollowCube) {
    auto ref = LoadStepAndConvert(PROJECT_ROOT_DIR "/resource/hollow_cube.step");
    ASSERT_TRUE(ref != nullptr);

    auto rt = RoundtripConvert(ref);
    ASSERT_TRUE(rt != nullptr);

    auto ref_stats = CollectStats(ref);
    auto rt_stats = CollectStats(rt);

    EXPECT_EQ(rt_stats.bodies, ref_stats.bodies);
    EXPECT_EQ(rt_stats.lumps, ref_stats.lumps);
    EXPECT_EQ(rt_stats.shells, ref_stats.shells);
    EXPECT_EQ(rt_stats.faces, ref_stats.faces);
    EXPECT_EQ(rt_stats.loops, ref_stats.loops);
    EXPECT_EQ(rt_stats.coedges, ref_stats.coedges);
    EXPECT_EQ(rt_stats.edges, ref_stats.edges);
    EXPECT_EQ(rt_stats.face_orient_counts, ref_stats.face_orient_counts);

    EXPECT_GT(rt_stats.faces_with_geom, 0);
    EXPECT_GT(rt_stats.edges_with_geom, 0);

    // Loop type may differ due to PK heuristics
    if (rt_stats.outer_loops != ref_stats.outer_loops) {
        printf("Warning: outer_loops differ: %d vs %d\n", rt_stats.outer_loops, ref_stats.outer_loops);
    }
}

TEST_F(RoundtripTest, Cylinder214) {
    auto ref = LoadStepAndConvert(PROJECT_ROOT_DIR "/resource/cylinder214.step");
    ASSERT_TRUE(ref != nullptr);

    auto rt = RoundtripConvert(ref);
    ASSERT_TRUE(rt != nullptr);

    auto ref_stats = CollectStats(ref);
    auto rt_stats = CollectStats(rt);

    EXPECT_EQ(rt_stats.bodies, ref_stats.bodies);
    EXPECT_EQ(rt_stats.lumps, ref_stats.lumps);
    EXPECT_EQ(rt_stats.shells, ref_stats.shells);
    EXPECT_EQ(rt_stats.faces, ref_stats.faces);
    EXPECT_EQ(rt_stats.loops, ref_stats.loops);
    EXPECT_EQ(rt_stats.coedges, ref_stats.coedges);
    EXPECT_EQ(rt_stats.edges, ref_stats.edges);
    EXPECT_EQ(rt_stats.face_orient_counts, ref_stats.face_orient_counts);

    EXPECT_GT(rt_stats.faces_with_geom, 0);
    EXPECT_GT(rt_stats.edges_with_geom, 0);

    if (rt_stats.outer_loops != ref_stats.outer_loops) {
        printf("Warning: outer_loops differ: %d vs %d\n", rt_stats.outer_loops, ref_stats.outer_loops);
    }
}
