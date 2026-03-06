#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cmath>


/**
 * @brief STEP 实体构建器
 * 用于构建符合 STEP AP214 语法的实体字符串
 * 参考 step_nio 的 SBuilder 实现
 */
class STEPEntityBuilder {
public:
    STEPEntityBuilder() = default;
    ~STEPEntityBuilder() = default;

    // 开始一个简单实体
    STEPEntityBuilder& BeginEntity(int id, const std::string& entityType);

    // 添加参数
    STEPEntityBuilder& AddString(const std::string& value);
    STEPEntityBuilder& AddReal(double value);
    STEPEntityBuilder& AddInteger(int value);
    STEPEntityBuilder& AddBoolean(bool value);
    STEPEntityBuilder& AddEnum(const std::string& enumValue);
    STEPEntityBuilder& AddEntityRef(int entityId);
    STEPEntityBuilder& AddEntityArray(const std::vector<int>& entityIds);
    STEPEntityBuilder& AddRealArray(const std::vector<double>& values);
    STEPEntityBuilder& AddIntegerArray(const std::vector<int>& values);
    STEPEntityBuilder& AddOptional();  // $
    STEPEntityBuilder& AddDerived();   // *

    // 构建并返回完整的 STEP 实体字符串
    std::string Build();

    // 重置构建器
    void Reset();

private:
    std::vector<std::string> m_buffer;
    bool m_firstParam = true;

    // 辅助函数
    static std::string FormatReal(double value);
    static std::string FormatString(const std::string& str);
    static std::string FormatEnum(const std::string& enumValue);
    static std::string FormatBoolean(bool value);
};


