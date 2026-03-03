#include <gtest/gtest.h>
#include "test_utils.hpp"
#include "pk_to_xchg_converter.hpp"

using namespace TestUtils;

// ============================================================================
// 测试夹具：自动管理 PK 会话
// ============================================================================

class RoundtripTest : public ::testing::Test {
protected:
    void SetUp() override {
        ASSERT_TRUE(InitializePK()) << "Failed to initialize Parasolid session";
    }

    void TearDown() override {
        TerminatePK();
    }

    // 执行完整的往返测试
    void ValidateRoundtrip(const std::string& step_file) {
        // 1. STEP → PK_BODY (原始)
        PK_BODY_t original = LoadStepFile(step_file);
        ASSERT_NE(original, PK_ENTITY_null) << "Failed to load STEP file: " << step_file;

        // 2. PK_BODY → Xchg_Body
        PKToXchgConverter converter;
        Xchg_BodyPtr xchg_body = nullptr;
        STEPExport_ErrorCode err = converter.Convert(original, &xchg_body);
        ASSERT_EQ(err, STEP_OK) << "Failed to convert PK_BODY to Xchg_Body";
        ASSERT_NE(xchg_body, nullptr) << "Xchg_Body is null after conversion";

        // 3. Xchg_Body → PK_BODY (往返)
        PK_BODY_t roundtrip = xchg_body->ConvertToPKBody();
        ASSERT_NE(roundtrip, PK_ENTITY_null) << "Failed to convert Xchg_Body back to PK_BODY";

        // 4. 验证拓扑一致性
        EXPECT_TRUE(CompareTopology(original, roundtrip))
            << "Topology mismatch for " << step_file;

        // 5. 验证几何一致性
        EXPECT_TRUE(CompareGeometry(original, roundtrip, 1e-6))
            << "Geometry mismatch for " << step_file;

        // 6. 打印统计信息（用于调试）
        int orig_faces = CountFaces(original);
        int orig_edges = CountEdges(original);
        int orig_vertices = CountVertices(original);
        int rt_faces = CountFaces(roundtrip);
        int rt_edges = CountEdges(roundtrip);
        int rt_vertices = CountVertices(roundtrip);

        std::cout << "=== " << step_file << " ===" << std::endl;
        std::cout << "Original:  Faces=" << orig_faces
                  << ", Edges=" << orig_edges
                  << ", Vertices=" << orig_vertices << std::endl;
        std::cout << "Roundtrip: Faces=" << rt_faces
                  << ", Edges=" << rt_edges
                  << ", Vertices=" << rt_vertices << std::endl;
    }
};

// ============================================================================
// 往返测试用例
// ============================================================================

TEST_F(RoundtripTest, HollowCube) {
    ValidateRoundtrip("hollow_cube.step");
}

TEST_F(RoundtripTest, Cube214) {
    ValidateRoundtrip("cube214.step");
}

TEST_F(RoundtripTest, Cylinder214) {
    ValidateRoundtrip("cylinder214.step");
}

// ============================================================================
// 拓扑一致性测试
// ============================================================================

TEST_F(RoundtripTest, TopologyConsistency_FaceCount) {
    PK_BODY_t original = LoadStepFile("hollow_cube.step");
    ASSERT_NE(original, PK_ENTITY_null);

    int pk_face_count1 = CountFaces(original);

    // PK → Xchg
    PKToXchgConverter converter;
    Xchg_BodyPtr xchg_body = nullptr;
    ASSERT_EQ(converter.Convert(original, &xchg_body), STEP_OK);

    int xchg_face_count = CountXchgFaces(xchg_body);

    // Xchg → PK
    PK_BODY_t roundtrip = xchg_body->ConvertToPKBody();
    int pk_face_count2 = CountFaces(roundtrip);

    // 验证：往返后 face 数量一致
    EXPECT_EQ(pk_face_count1, xchg_face_count)
        << "Face count mismatch: PK=" << pk_face_count1
        << ", Xchg=" << xchg_face_count;
    EXPECT_EQ(pk_face_count1, pk_face_count2)
        << "Face count mismatch after roundtrip: Original=" << pk_face_count1
        << ", Roundtrip=" << pk_face_count2;
}

TEST_F(RoundtripTest, TopologyConsistency_EdgeCount) {
    PK_BODY_t original = LoadStepFile("cube214.step");
    ASSERT_NE(original, PK_ENTITY_null);

    int pk_edge_count1 = CountEdges(original);

    // PK → Xchg
    PKToXchgConverter converter;
    Xchg_BodyPtr xchg_body = nullptr;
    ASSERT_EQ(converter.Convert(original, &xchg_body), STEP_OK);

    int xchg_edge_count = CountUniqueXchgEdges(xchg_body);

    // Xchg → PK
    PK_BODY_t roundtrip = xchg_body->ConvertToPKBody();
    int pk_edge_count2 = CountEdges(roundtrip);

    // 验证：往返后 edge 数量一致
    EXPECT_EQ(pk_edge_count1, xchg_edge_count)
        << "Edge count mismatch: PK=" << pk_edge_count1
        << ", Xchg=" << xchg_edge_count;
    EXPECT_EQ(pk_edge_count1, pk_edge_count2)
        << "Edge count mismatch after roundtrip: Original=" << pk_edge_count1
        << ", Roundtrip=" << pk_edge_count2;
}

// ============================================================================
// 几何精度测试
// ============================================================================

TEST_F(RoundtripTest, GeometryAccuracy_VertexPositions) {
    PK_BODY_t original = LoadStepFile("cube214.step");
    ASSERT_NE(original, PK_ENTITY_null);

    // 提取原始顶点坐标
    std::vector<PK_VECTOR_t> original_vertices = ExtractVertices(original);
    ASSERT_GT(original_vertices.size(), 0) << "No vertices found in original body";

    // 往返转换
    PKToXchgConverter converter;
    Xchg_BodyPtr xchg_body = nullptr;
    ASSERT_EQ(converter.Convert(original, &xchg_body), STEP_OK);

    PK_BODY_t roundtrip = xchg_body->ConvertToPKBody();
    ASSERT_NE(roundtrip, PK_ENTITY_null);

    // 提取转换后顶点坐标
    std::vector<PK_VECTOR_t> roundtrip_vertices = ExtractVertices(roundtrip);

    // 验证：顶点数量一致
    ASSERT_EQ(original_vertices.size(), roundtrip_vertices.size())
        << "Vertex count mismatch";

    // 验证：坐标误差 < 1e-6
    for (size_t i = 0; i < original_vertices.size(); ++i) {
        EXPECT_NEAR(original_vertices[i].coord[0], roundtrip_vertices[i].coord[0], 1e-6)
            << "Vertex " << i << " X coordinate mismatch";
        EXPECT_NEAR(original_vertices[i].coord[1], roundtrip_vertices[i].coord[1], 1e-6)
            << "Vertex " << i << " Y coordinate mismatch";
        EXPECT_NEAR(original_vertices[i].coord[2], roundtrip_vertices[i].coord[2], 1e-6)
            << "Vertex " << i << " Z coordinate mismatch";
    }
}

TEST_F(RoundtripTest, GeometryAccuracy_BoundingBox) {
    PK_BODY_t original = LoadStepFile("cylinder214.step");
    ASSERT_NE(original, PK_ENTITY_null);

    // 计算原始包围盒
    BoundingBox bbox1 = ComputeBoundingBox(original);
    ASSERT_TRUE(bbox1.IsValid()) << "Invalid bounding box for original body";

    // 往返转换
    PKToXchgConverter converter;
    Xchg_BodyPtr xchg_body = nullptr;
    ASSERT_EQ(converter.Convert(original, &xchg_body), STEP_OK);

    PK_BODY_t roundtrip = xchg_body->ConvertToPKBody();
    ASSERT_NE(roundtrip, PK_ENTITY_null);

    // 计算往返后包围盒
    BoundingBox bbox2 = ComputeBoundingBox(roundtrip);
    ASSERT_TRUE(bbox2.IsValid()) << "Invalid bounding box for roundtrip body";

    // 验证：包围盒一致（误差 < 1e-6）
    EXPECT_NEAR(bbox1.min_x, bbox2.min_x, 1e-6) << "Bounding box min_x mismatch";
    EXPECT_NEAR(bbox1.min_y, bbox2.min_y, 1e-6) << "Bounding box min_y mismatch";
    EXPECT_NEAR(bbox1.min_z, bbox2.min_z, 1e-6) << "Bounding box min_z mismatch";
    EXPECT_NEAR(bbox1.max_x, bbox2.max_x, 1e-6) << "Bounding box max_x mismatch";
    EXPECT_NEAR(bbox1.max_y, bbox2.max_y, 1e-6) << "Bounding box max_y mismatch";
    EXPECT_NEAR(bbox1.max_z, bbox2.max_z, 1e-6) << "Bounding box max_z mismatch";
}

// ============================================================================
// 实体去重验证
// ============================================================================

TEST_F(RoundtripTest, Deduplication_SharedEdges) {
    // 立方体有 12 条边，每条边被 2 个面共享
    PK_BODY_t original = LoadStepFile("cube214.step");
    ASSERT_NE(original, PK_ENTITY_null);

    PKToXchgConverter converter;
    Xchg_BodyPtr xchg_body = nullptr;
    ASSERT_EQ(converter.Convert(original, &xchg_body), STEP_OK);

    // 统计 Xchg_Edge 数量
    int unique_edge_count = CountUniqueXchgEdges(xchg_body);

    // 验证：立方体应该有 12 条唯一边
    EXPECT_EQ(unique_edge_count, 12)
        << "Expected 12 unique edges in cube, got " << unique_edge_count;
}

TEST_F(RoundtripTest, Deduplication_SharedVertices) {
    // 立方体有 8 个顶点，每个顶点被多条边共享
    PK_BODY_t original = LoadStepFile("cube214.step");
    ASSERT_NE(original, PK_ENTITY_null);

    PKToXchgConverter converter;
    Xchg_BodyPtr xchg_body = nullptr;
    ASSERT_EQ(converter.Convert(original, &xchg_body), STEP_OK);

    // 统计 Xchg_Vertex 数量
    int unique_vertex_count = CountUniqueXchgVertices(xchg_body);

    // 验证：立方体应该有 8 个唯一顶点
    EXPECT_EQ(unique_vertex_count, 8)
        << "Expected 8 unique vertices in cube, got " << unique_vertex_count;
}

// ============================================================================
// 主函数
// ============================================================================

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
