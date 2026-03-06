#include "../include/xchg_to_step_writer.hpp"
#include "../include/step_entity_builder.hpp"
#include "../include/xchg_entity_mapper.hpp"

#include "xchg_maindoc.hpp"
#include "xchg_componentinstance.hpp"
#include "xchg_component.hpp"
#include "xchg_node.hpp"
#include "topology/xchg_body.hpp"
#include "topology/xchg_lump.hpp"
#include "topology/xchg_shell.hpp"
#include "topology/xchg_face.hpp"
#include "topology/xchg_loop.hpp"
#include "topology/xchg_coedge.hpp"
#include "topology/xchg_edge.hpp"
#include "topology/xchg_vertex.hpp"
#include "geom/surface/xchg_surface.hpp"
#include "geom/curve/xchg_curve.hpp"
#include "geom/xchg_point.hpp"

#include <iostream>
#include <ctime>



XchgToSTEPWriter::XchgToSTEPWriter(const std::string& filepath)
    : m_fileStream(std::make_unique<std::ofstream>(filepath))
    , m_output(m_fileStream.get())
    , m_ownsStream(true)
    , m_builder(std::make_unique<STEPEntityBuilder>())
    , m_mapper(std::make_unique<XchgEntityMapper>())
    , m_lengthUnit(STEPLengthUnit::Millimeter)
    , m_uncertainty(1e-6)
    , m_applicationContextId(0)
    , m_geometricRepContextId(0)
    , m_globalUncertaintyId(0)
    , m_globalUnitId(0)
    , m_axis2Placement3DId(0)
    , m_initialized(false)
    , m_done(false)
{
    if (!m_fileStream->is_open()) {
        std::cerr << "[Error] Failed to open file: " << filepath << std::endl;
        m_output = nullptr;
    }
}

XchgToSTEPWriter::XchgToSTEPWriter(std::ostream& os)
    : m_fileStream(nullptr)
    , m_output(&os)
    , m_ownsStream(false)
    , m_builder(std::make_unique<STEPEntityBuilder>())
    , m_mapper(std::make_unique<XchgEntityMapper>())
    , m_lengthUnit(STEPLengthUnit::Millimeter)
    , m_uncertainty(1e-6)
    , m_applicationContextId(0)
    , m_geometricRepContextId(0)
    , m_globalUncertaintyId(0)
    , m_globalUnitId(0)
    , m_axis2Placement3DId(0)
    , m_initialized(false)
    , m_done(false)
{
}

XchgToSTEPWriter::~XchgToSTEPWriter() {
    if (!m_done) {
        Done();
    }
}

void XchgToSTEPWriter::SetLengthUnit(STEPLengthUnit unit) {
    m_lengthUnit = unit;
}

void XchgToSTEPWriter::SetPrecision(double uncertainty) {
    m_uncertainty = uncertainty;
}

bool XchgToSTEPWriter::WriteMainDoc(const Xchg_MainDocPtr& mainDoc) {
    if (!m_output) {
        std::cerr << "[Error] Output stream is not valid" << std::endl;
        return false;
    }

    if (!mainDoc) {
        std::cerr << "[Error] MainDoc is null" << std::endl;
        return false;
    }

    // 写出文件头部
    if (!m_initialized) {
        WriteHeader();
        WriteGlobalContext();
        m_initialized = true;
    }

    // 获取根组件
    const Xchg_ComponentPtr& rootComp = mainDoc->RootComponent();
    if (!rootComp) {
        std::cerr << "[Error] MainDoc has no root component" << std::endl;
        return false;
    }

    // 写出组件树
    WriteComponent(rootComp);

    return true;
}

bool XchgToSTEPWriter::Done() {
    if (m_done) {
        return true;
    }

    if (!m_output) {
        return false;
    }

    // 写出文件尾部
    WriteFooter();

    m_done = true;
    return true;
}

void XchgToSTEPWriter::WriteHeader() {
    if (!m_output) return;

    // ISO-10303-21 头部
    *m_output << "ISO-10303-21;\n";
    *m_output << "HEADER;\n";

    // FILE_DESCRIPTION
    *m_output << "FILE_DESCRIPTION(('STEP AP214'),'2;1');\n";

    // FILE_NAME
    std::time_t now = std::time(nullptr);
    char timeStr[64];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%dT%H:%M:%S", std::localtime(&now));

    *m_output << "FILE_NAME('','";
    *m_output << timeStr;
    *m_output << "',(''),(''),'PKToSTEP','PKToSTEP','');\n";

    // FILE_SCHEMA
    *m_output << "FILE_SCHEMA(('AUTOMOTIVE_DESIGN'));\n";
    *m_output << "ENDSEC;\n";

    // DATA 段开始
    *m_output << "DATA;\n";
}

void XchgToSTEPWriter::WriteFooter() {
    if (!m_output) return;

    *m_output << "ENDSEC;\n";
    *m_output << "END-ISO-10303-21;\n";
}

void XchgToSTEPWriter::WriteGlobalContext() {
    // APPLICATION_CONTEXT
    m_applicationContextId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(m_applicationContextId, "APPLICATION_CONTEXT")
        .AddString("automotive_design")
        .Build();
    WriteEntity(entity);

    // GEOMETRIC_REPRESENTATION_CONTEXT
    m_geometricRepContextId = WriteGeometricRepresentationContext();

    // GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT
    m_globalUncertaintyId = WriteGlobalUncertaintyAssignedContext();

    // GLOBAL_UNIT_ASSIGNED_CONTEXT
    m_globalUnitId = WriteGlobalUnitAssignedContext();

    // 全局坐标系 AXIS2_PLACEMENT_3D
    int originId = WritePoint(nullptr);  // (0,0,0)
    int zDirId = WriteDirection(0, 0, 1);
    int xDirId = WriteDirection(1, 0, 0);

    m_axis2Placement3DId = m_mapper->AllocateNewId();
    entity = m_builder->BeginEntity(m_axis2Placement3DId, "AXIS2_PLACEMENT_3D")
        .AddString("")
        .AddEntityRef(originId)
        .AddEntityRef(zDirId)
        .AddEntityRef(xDirId)
        .Build();
    WriteEntity(entity);
}

int XchgToSTEPWriter::WriteGeometricRepresentationContext() {
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "GEOMETRIC_REPRESENTATION_CONTEXT")
        .AddInteger(3)
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteGlobalUncertaintyAssignedContext() {
    // UNCERTAINTY_MEASURE_WITH_UNIT
    int uncertaintyMeasureId = m_mapper->AllocateNewId();

    // LENGTH_UNIT
    int lengthUnitId = m_mapper->AllocateNewId();
    std::string lengthUnitEntity = m_builder->BeginEntity(lengthUnitId, "(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT(.MILLI.,.METRE.))")
        .Build();
    // 这是复合实体，需要特殊处理
    *m_output << "#" << lengthUnitId << "=(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT(.MILLI.,.METRE.));\n";

    std::string entity = m_builder->BeginEntity(uncertaintyMeasureId, "UNCERTAINTY_MEASURE_WITH_UNIT")
        .AddReal(m_uncertainty)
        .AddEntityRef(lengthUnitId)
        .AddString("distance_accuracy_value")
        .AddString("confusion accuracy")
        .Build();
    WriteEntity(entity);

    // GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT
    int id = m_mapper->AllocateNewId();
    entity = m_builder->BeginEntity(id, "GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT")
        .AddEntityArray({uncertaintyMeasureId})
        .Build();
    WriteEntity(entity);

    return id;
}

int XchgToSTEPWriter::WriteGlobalUnitAssignedContext() {
    // LENGTH_UNIT
    int lengthUnitId = m_mapper->AllocateNewId();
    std::string unitPrefix = ".MILLI.";
    if (m_lengthUnit == STEPLengthUnit::Meter) {
        unitPrefix = "$";
    } else if (m_lengthUnit == STEPLengthUnit::Inch) {
        unitPrefix = "$";
    }
    *m_output << "#" << lengthUnitId << "=(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT(" << unitPrefix << ",.METRE.));\n";

    // PLANE_ANGLE_UNIT
    int angleUnitId = m_mapper->AllocateNewId();
    *m_output << "#" << angleUnitId << "=(NAMED_UNIT(*)PLANE_ANGLE_UNIT()SI_UNIT($,.RADIAN.));\n";

    // SOLID_ANGLE_UNIT
    int solidAngleUnitId = m_mapper->AllocateNewId();
    *m_output << "#" << solidAngleUnitId << "=(NAMED_UNIT(*)SI_UNIT($,.STERADIAN.)SOLID_ANGLE_UNIT());\n";

    // GLOBAL_UNIT_ASSIGNED_CONTEXT
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "GLOBAL_UNIT_ASSIGNED_CONTEXT")
        .AddEntityArray({lengthUnitId, angleUnitId, solidAngleUnitId})
        .Build();
    WriteEntity(entity);

    return id;
}

int XchgToSTEPWriter::WriteProduct(const std::string& name) {
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "PRODUCT")
        .AddString("")
        .AddString(name)
        .AddString("")
        .AddEntityArray({})
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteProductDefinitionFormation(int productId) {
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "PRODUCT_DEFINITION_FORMATION")
        .AddString("")
        .AddString("")
        .AddEntityRef(productId)
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteProductDefinition(int productDefFormationId) {
    int contextId = WriteProductDefinitionContext(m_applicationContextId);

    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "PRODUCT_DEFINITION")
        .AddString("")
        .AddString("")
        .AddEntityRef(productDefFormationId)
        .AddEntityRef(contextId)
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteProductDefinitionContext(int applicationContextId) {
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "PRODUCT_DEFINITION_CONTEXT")
        .AddString("part definition")
        .AddEntityRef(applicationContextId)
        .AddString("manufacturing")
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteProductDefinitionShape(int productDefId) {
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "PRODUCT_DEFINITION_SHAPE")
        .AddString("")
        .AddString("")
        .AddEntityRef(productDefId)
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteShapeDefinitionRepresentation(int productDefShapeId, int shapeRepId) {
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "SHAPE_DEFINITION_REPRESENTATION")
        .AddEntityRef(productDefShapeId)
        .AddEntityRef(shapeRepId)
        .Build();
    WriteEntity(entity);
    return id;
}

void XchgToSTEPWriter::WriteComponent(const Xchg_ComponentPtr& comp) {
    if (!comp) return;

    // 写出产品结构
    std::string compName;
    if (compName.empty()) {
        compName = "Component";
    }

    int productId = WriteProduct(compName);
    int productDefFormationId = WriteProductDefinitionFormation(productId);
    int productDefId = WriteProductDefinition(productDefFormationId);
    int productDefShapeId = WriteProductDefinitionShape(productDefId);

    // 收集所有 body 的 shape representation
    std::vector<int> bodyRepIds;
    Xchg_Size_t numNodes = comp->GetNumNodes();
    for (Xchg_Size_t i = 0; i < numNodes; ++i) {
        Xchg_NodePtr node = comp->GetNodeByIndex(i);
        if (!node) continue;

        Xchg_BodyPtr body = node->GetBodyPtr();
        if (body) {
            int bodyRepId = WriteBodyAsShapeRepresentation(body);
            if (bodyRepId > 0) {
                bodyRepIds.push_back(bodyRepId);
            }
        }
    }

    // 创建主 SHAPE_REPRESENTATION，包含所有 body 和全局坐标系
    int mainShapeRepId = m_mapper->AllocateNewId();
    std::vector<int> repItems = {m_axis2Placement3DId};
    repItems.insert(repItems.end(), bodyRepIds.begin(), bodyRepIds.end());

    std::string entity = m_builder->BeginEntity(mainShapeRepId, "SHAPE_REPRESENTATION")
        .AddString("")
        .AddEntityArray(repItems)
        .AddEntityRef(m_geometricRepContextId)
        .Build();
    WriteEntity(entity);

    // 关联产品和几何
    WriteShapeDefinitionRepresentation(productDefShapeId, mainShapeRepId);

    // 递归处理子组件（装配结构）
    // TODO: 实现装配结构的 NAUO + CDSR 写出
}

int XchgToSTEPWriter::WriteBodyAsShapeRepresentation(const Xchg_BodyPtr& body) {
    if (!body) return 0;

    // 写出 Body
    int bodyId = WriteBody(body);

    // 创建 SHAPE_REPRESENTATION
    int shapeRepId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(shapeRepId, "ADVANCED_BREP_SHAPE_REPRESENTATION")
        .AddString("")
        .AddEntityArray({bodyId, m_axis2Placement3DId})
        .AddEntityRef(m_geometricRepContextId)
        .Build();
    WriteEntity(entity);

    return shapeRepId;
}

void XchgToSTEPWriter::WriteEntity(const std::string& entity) {
    if (m_output && !entity.empty()) {
        *m_output << entity;
    }
}

std::string XchgToSTEPWriter::GetLengthUnitString() const {
    switch (m_lengthUnit) {
        case STEPLengthUnit::Millimeter: return "MM";
        case STEPLengthUnit::Meter: return "M";
        case STEPLengthUnit::Inch: return "INCH";
        default: return "MM";
    }
}

double XchgToSTEPWriter::GetLengthUnitFactor() const {
    switch (m_lengthUnit) {
        case STEPLengthUnit::Millimeter: return 1.0;
        case STEPLengthUnit::Meter: return 1000.0;
        case STEPLengthUnit::Inch: return 25.4;
        default: return 1.0;
    }
}

int XchgToSTEPWriter::WritePoint(const Xchg_PointPtr& point) {
    // 如果 point 为空，写出原点
    double x = 0.0, y = 0.0, z = 0.0;
    if (point) {
        x = point->x();
        y = point->y();
        z = point->z();
    }

    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "CARTESIAN_POINT")
        .AddString("")
        .AddRealArray({x, y, z})
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteDirection(double x, double y, double z) {
    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "DIRECTION")
        .AddString("")
        .AddRealArray({x, y, z})
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteVector(double x, double y, double z, double magnitude) {
    int dirId = WriteDirection(x, y, z);

    int id = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(id, "VECTOR")
        .AddString("")
        .AddEntityRef(dirId)
        .AddReal(magnitude)
        .Build();
    WriteEntity(entity);
    return id;
}

int XchgToSTEPWriter::WriteAxis2Placement3D(double ox, double oy, double oz,
                                             double zx, double zy, double zz,
                                             double xx, double xy, double xz) {
    int originId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(originId, "CARTESIAN_POINT")
        .AddString("")
        .AddRealArray({ox, oy, oz})
        .Build();
    WriteEntity(entity);

    int zDirId = WriteDirection(zx, zy, zz);
    int xDirId = WriteDirection(xx, xy, xz);

    int id = m_mapper->AllocateNewId();
    entity = m_builder->BeginEntity(id, "AXIS2_PLACEMENT_3D")
        .AddString("")
        .AddEntityRef(originId)
        .AddEntityRef(zDirId)
        .AddEntityRef(xDirId)
        .Build();
    WriteEntity(entity);

    return id;
}


