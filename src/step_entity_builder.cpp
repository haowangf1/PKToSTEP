#include "../include/step_entity_builder.hpp"
#include <cstdio>
#include <limits>


STEPEntityBuilder& STEPEntityBuilder::BeginEntity(int id, const std::string& entityType) {
    m_buffer.clear();
    m_firstParam = true;

    std::string header = "#" + std::to_string(id) + "=" + entityType;
    m_buffer.push_back(header);

    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddString(const std::string& value) {
    m_buffer.push_back(FormatString(value));
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddReal(double value) {
    m_buffer.push_back(FormatReal(value));
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddInteger(int value) {
    m_buffer.push_back(std::to_string(value));
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddBoolean(bool value) {
    m_buffer.push_back(FormatBoolean(value));
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddEnum(const std::string& enumValue) {
    m_buffer.push_back(FormatEnum(enumValue));
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddEntityRef(int entityId) {
    m_buffer.push_back("#" + std::to_string(entityId));
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddEntityArray(const std::vector<int>& entityIds) {
    std::string result = "(";
    for (size_t i = 0; i < entityIds.size(); ++i) {
        if (i > 0) result += ",";
        result += "#" + std::to_string(entityIds[i]);
    }
    result += ")";
    m_buffer.push_back(result);
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddRealArray(const std::vector<double>& values) {
    std::string result = "(";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) result += ",";
        result += FormatReal(values[i]);
    }
    result += ")";
    m_buffer.push_back(result);
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddIntegerArray(const std::vector<int>& values) {
    std::string result = "(";
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) result += ",";
        result += std::to_string(values[i]);
    }
    result += ")";
    m_buffer.push_back(result);
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddOptional() {
    m_buffer.push_back("$");
    return *this;
}

STEPEntityBuilder& STEPEntityBuilder::AddDerived() {
    m_buffer.push_back("*");
    return *this;
}

std::string STEPEntityBuilder::Build() {
    if (m_buffer.empty()) {
        return "";
    }

    std::string result = m_buffer[0];  // #id=ENTITY_TYPE
    result += "(";

    for (size_t i = 1; i < m_buffer.size(); ++i) {
        if (i > 1) result += ",";
        result += m_buffer[i];
    }

    result += ");\n";
    return result;
}

void STEPEntityBuilder::Reset() {
    m_buffer.clear();
    m_firstParam = true;
}

std::string STEPEntityBuilder::FormatReal(double value) {
    // 处理特殊值
    if (std::isnan(value) || std::isinf(value)) {
        return "0.0";
    }

    // 检查是否接近整数
    double rounded = std::round(value);
    if (std::fabs(value - rounded) < 1e-10) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.1f", rounded);
        return std::string(buffer);
    }

    // 使用科学计数法或固定小数点
    double absVal = std::fabs(value);
    char buffer[64];

    if (absVal >= 1.0 && absVal <= 10.0) {
        // 小范围使用固定小数点
        snprintf(buffer, sizeof(buffer), "%.13f", value);
    } else {
        // 其他使用科学计数法
        snprintf(buffer, sizeof(buffer), "%.13E", value);
    }

    return std::string(buffer);
}

std::string STEPEntityBuilder::FormatString(const std::string& str) {
    std::string result = "'";
    for (char c : str) {
        result += c;
        if (c == '\'') {
            result += '\'';  // 单引号需要转义
        }
    }
    result += "'";
    return result;
}

std::string STEPEntityBuilder::FormatEnum(const std::string& enumValue) {
    return "." + enumValue + ".";
}

std::string STEPEntityBuilder::FormatBoolean(bool value) {
    return value ? ".T." : ".F.";
}
