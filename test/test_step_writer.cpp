#include <gtest/gtest.h>
#include "../include/xchg_to_step_writer.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_maindoc.hpp"
#include "step/translator_step.hpp"

#include <fstream>
#include <string>


class STEPWriterTest : public ::testing::Test {
protected:
    void SetUp() override {
        // 初始化 STEP 读取上下文
        AMXT_STP_CTX_create(&ctx);
    }

    void TearDown() override {
        if (ctx) {
            AMXT_STP_CTX_destory(ctx);
        }
    }

    AMXT_STP_CTX_context_p ctx = nullptr;
};

TEST_F(STEPWriterTest, WriteSimpleCube) {
    // 读取测试 STEP 文件
    std::string inputFile = std::string(PROJECT_ROOT_DIR) + "/resource/cube214.step";

    AMXT_STP_read_o_t read_opts;
    AMXT_STP_read_o_m(read_opts);

    Xchg_MainDocPtr mainDoc = Xchg_MainDoc::Create();
    AMXT_STP_ERROR_code_t err = AMXT_STP_read(ctx, inputFile.c_str(), &read_opts, &mainDoc);

    ASSERT_EQ(err, AMXT_STP_ERROR_no_errors) << "Failed to read input STEP file";
    ASSERT_TRUE(mainDoc) << "MainDoc is null";

    // 写出 STEP 文件
    std::string outputFile = std::string(PROJECT_ROOT_DIR) + "/build/test_output_cube.step";

    XchgToSTEPWriter writer(outputFile);
    writer.SetLengthUnit(STEPLengthUnit::Millimeter);
    writer.SetPrecision(1e-6);

    bool success = writer.WriteMainDoc(mainDoc);
    ASSERT_TRUE(success) << "Failed to write STEP file";

    success = writer.Done();
    ASSERT_TRUE(success) << "Failed to finalize STEP file";

    // 验证输出文件存在
    std::ifstream checkFile(outputFile);
    ASSERT_TRUE(checkFile.good()) << "Output file does not exist";
    checkFile.close();

    std::cout << "[Info] STEP file written successfully: " << outputFile << std::endl;
}

TEST_F(STEPWriterTest, WriteHollowCube) {
    // 读取带空腔的立方体
    std::string inputFile = std::string(PROJECT_ROOT_DIR) + "/resource/hollow_cube.step";

    AMXT_STP_read_o_t read_opts;
    AMXT_STP_read_o_m(read_opts);

    Xchg_MainDocPtr mainDoc = Xchg_MainDoc::Create();
    AMXT_STP_ERROR_code_t err = AMXT_STP_read(ctx, inputFile.c_str(), &read_opts, &mainDoc);

    ASSERT_EQ(err, AMXT_STP_ERROR_no_errors) << "Failed to read input STEP file";
    ASSERT_TRUE(mainDoc) << "MainDoc is null";

    // 写出 STEP 文件
    std::string outputFile = std::string(PROJECT_ROOT_DIR) + "/build/test_output_hollow_cube.step";

    XchgToSTEPWriter writer(outputFile);
    writer.SetLengthUnit(STEPLengthUnit::Millimeter);
    writer.SetPrecision(1e-6);

    bool success = writer.WriteMainDoc(mainDoc);
    ASSERT_TRUE(success) << "Failed to write STEP file";

    success = writer.Done();
    ASSERT_TRUE(success) << "Failed to finalize STEP file";

    // 验证输出文件存在
    std::ifstream checkFile(outputFile);
    ASSERT_TRUE(checkFile.good()) << "Output file does not exist";
    checkFile.close();

    std::cout << "[Info] STEP file written successfully: " << outputFile << std::endl;
}

TEST_F(STEPWriterTest, WriteCylinder) {
    // 读取圆柱体
    std::string inputFile = std::string(PROJECT_ROOT_DIR) + "/resource/cylinder214.step";

    AMXT_STP_read_o_t read_opts;
    AMXT_STP_read_o_m(read_opts);

    Xchg_MainDocPtr mainDoc = Xchg_MainDoc::Create();
    AMXT_STP_ERROR_code_t err = AMXT_STP_read(ctx, inputFile.c_str(), &read_opts, &mainDoc);

    ASSERT_EQ(err, AMXT_STP_ERROR_no_errors) << "Failed to read input STEP file";
    ASSERT_TRUE(mainDoc) << "MainDoc is null";

    // 写出 STEP 文件
    std::string outputFile = std::string(PROJECT_ROOT_DIR) + "/build/test_output_cylinder.step";

    XchgToSTEPWriter writer(outputFile);
    writer.SetLengthUnit(STEPLengthUnit::Millimeter);
    writer.SetPrecision(1e-6);

    bool success = writer.WriteMainDoc(mainDoc);
    ASSERT_TRUE(success) << "Failed to write STEP file";

    success = writer.Done();
    ASSERT_TRUE(success) << "Failed to finalize STEP file";

    // 验证输出文件存在
    std::ifstream checkFile(outputFile);
    ASSERT_TRUE(checkFile.good()) << "Output file does not exist";
    checkFile.close();

    std::cout << "[Info] STEP file written successfully: " << outputFile << std::endl;
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
