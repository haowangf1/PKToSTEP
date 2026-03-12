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

    // Xchg 格式内部长度单位固定为【米】（SI base unit）。
    // 原因：amxt_stp 读取器在 GeomBuilder.cpp 中将 STEP 文件的长度值乘以
    // m_length_factor（例如毫米文件的 factor=0.001）统一转换为米存入 Xchg。
    // 因此写出时单位声明必须是 SI_UNIT($,.METRE.)，数值才与单位声明一致。
    // 不能依赖 m_lengthUnit 设置，因为 Xchg 内部单位始终是米，与调用方设置无关。

    // LENGTH_UNIT: 米（SI base, no prefix）
    int lengthUnitId = m_mapper->AllocateNewId();
    *m_output << "#" << lengthUnitId << "=(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT($,.METRE.));\n";

    // PLANE_ANGLE_UNIT: 弧度（SI base）
    int angleUnitId = m_mapper->AllocateNewId();
    *m_output << "#" << angleUnitId << "=(NAMED_UNIT(*)PLANE_ANGLE_UNIT()SI_UNIT($,.RADIAN.));\n";

    // SOLID_ANGLE_UNIT: 球面度（SI base）
    int solidAngleUnitId = m_mapper->AllocateNewId();
    *m_output << "#" << solidAngleUnitId << "=(NAMED_UNIT(*)SI_UNIT($,.STERADIAN.)SOLID_ANGLE_UNIT());\n";

    // UNCERTAINTY_MEASURE_WITH_UNIT：精度值单位与长度单位一致（米）
    // m_uncertainty 默认 1e-6 m = 1 纳米，合理
    int uncertaintyMeasureId = m_mapper->AllocateNewId();
    entity = m_builder->BeginEntity(uncertaintyMeasureId, "UNCERTAINTY_MEASURE_WITH_UNIT")
        .AddReal(m_uncertainty)
        .AddEntityRef(lengthUnitId)
        .AddString("distance_accuracy_value")
        .AddString("confusion accuracy")
        .Build();
    WriteEntity(entity);

    // 复合上下文实体（gggr）：将几何上下文、单位、精度合并为一个实体。
    // ADVANCED_BREP_SHAPE_REPRESENTATION 的第三个参数引用此实体。
    // 参考 step_nio::STEPWriter_Actor::writeGGGR() 的做法。
    m_geometricRepContextId = m_mapper->AllocateNewId();
    *m_output << "#" << m_geometricRepContextId
              << "=( GEOMETRIC_REPRESENTATION_CONTEXT(3) "
              << "GLOBAL_UNCERTAINTY_ASSIGNED_CONTEXT((#" << uncertaintyMeasureId << ")) "
              << "GLOBAL_UNIT_ASSIGNED_CONTEXT((#" << lengthUnitId
              << ",#" << angleUnitId
              << ",#" << solidAngleUnitId << ")) "
              << "REPRESENTATION_CONTEXT('','') );\n";

    // 全局坐标系 AXIS2_PLACEMENT_3D（原点，Z轴朝上，X轴朝右）
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

    // LENGTH_UNIT (metre - Xchg内部单位是米)
    int lengthUnitId = m_mapper->AllocateNewId();
    // 这是复合实体，需要特殊处理
    *m_output << "#" << lengthUnitId << "=(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT($,.METRE.));\n";

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
    // LENGTH_UNIT (metre - Xchg内部单位是米，直接用 SI_UNIT($,.METRE.))
    int lengthUnitId = m_mapper->AllocateNewId();
    *m_output << "#" << lengthUnitId << "=(LENGTH_UNIT()NAMED_UNIT(*)SI_UNIT($,.METRE.));\n";

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

// 从 Xchg_transfo 写出 AXIS2_PLACEMENT_3D，返回实体 ID
int XchgToSTEPWriter::WriteAxis2Placement3DFromTransfo(const Xchg_transfo& trsf) {
    const Xchg_pnt& o  = trsf.getOrigin();
    const Xchg_dir& xd = trsf.getXdir();
    const Xchg_dir& zd = trsf.getZdir();
    return WriteAxis2Placement3D(
        o.x(), o.y(), o.z(),
        zd.x(), zd.y(), zd.z(),
        xd.x(), xd.y(), xd.z());
}

// 写出父子装配关系
// 参考 step_nio STEPWriter_Actor::writeCDSR / writeItemDefinedTransformation
void XchgToSTEPWriter::WriteAssemblyLink(int parentSrId, int parentPdId,
                                          int childSrId,  int childPdId,
                                          const Xchg_transfo& trsf) {
    // AXIS2_PLACEMENT_3D for child placement
    int childPlacementId = WriteAxis2Placement3DFromTransfo(trsf);

    // ITEM_DEFINED_TRANSFORMATION
    int idtId = m_mapper->AllocateNewId();
    WriteEntity("#" + std::to_string(idtId) +
        "=ITEM_DEFINED_TRANSFORMATION(\'\',\'\',#" +
        std::to_string(m_axis2Placement3DId) + ",#" +
        std::to_string(childPlacementId) + ");\n");

    // REPRESENTATION_RELATIONSHIP_WITH_TRANSFORMATION (complex entity)
    int srrwtId = m_mapper->AllocateNewId();
    WriteEntity("#" + std::to_string(srrwtId) +
        "=(REPRESENTATION_RELATIONSHIP(\'\',\'\',#" +
        std::to_string(childSrId) + ",#" +
        std::to_string(parentSrId) +
        ")REPRESENTATION_RELATIONSHIP_WITH_TRANSFORMATION(#" +
        std::to_string(idtId) +
        ")SHAPE_REPRESENTATION_RELATIONSHIP());\n");

    // NEXT_ASSEMBLY_USAGE_OCCURRENCE
    int nauoId = m_mapper->AllocateNewId();
    WriteEntity("#" + std::to_string(nauoId) +
        "=NEXT_ASSEMBLY_USAGE_OCCURRENCE(\'\',\'\',\'\',#" +
        std::to_string(parentPdId) + ",#" +
        std::to_string(childPdId) + ",$);\n");

    // PRODUCT_DEFINITION_SHAPE referencing NAUO directly
    int nauoPdsId = m_mapper->AllocateNewId();
    WriteEntity("#" + std::to_string(nauoPdsId) +
        "=PRODUCT_DEFINITION_SHAPE(\'Placement\',\'Placement of an item\',#" +
        std::to_string(nauoId) + ");\n");

    // CONTEXT_DEPENDENT_SHAPE_REPRESENTATION
    int cdsrId = m_mapper->AllocateNewId();
    WriteEntity("#" + std::to_string(cdsrId) +
        "=CONTEXT_DEPENDENT_SHAPE_REPRESENTATION(#" +
        std::to_string(srrwtId) + ",#" +
        std::to_string(nauoPdsId) + ");\n");
}
XchgToSTEPWriter::ComponentIds XchgToSTEPWriter::WriteComponent(const Xchg_ComponentPtr& comp) {
    ComponentIds result{0, 0};
    if (!comp) return result;

    // 产品名称
    std::string compName = comp->Name().c_str();
    if (compName.empty()) compName = "Component";

    int productId          = WriteProduct(compName);
    int productDefFormId   = WriteProductDefinitionFormation(productId);
    int productDefId       = WriteProductDefinition(productDefFormId);
    int productDefShapeId  = WriteProductDefinitionShape(productDefId);

    // 写出当前组件的几何（NodesPool 中所有 body）
    std::vector<int> bodyIds;
    Xchg_Size_t numNodes = comp->GetNumNodes();
    for (Xchg_Size_t i = 0; i < numNodes; ++i) {
        Xchg_NodePtr node = comp->GetNodeByIndex(i);
        if (!node) continue;
        Xchg_BodyPtr body = node->GetBodyPtr();
        if (body) {
            int bodyId = WriteBody(body);
            if (bodyId > 0) bodyIds.push_back(bodyId);
        }
    }

    // 所有 body 放入同一个 ADVANCED_BREP_SHAPE_REPRESENTATION
    // 双层结构：SDR→SR，SR→ABSR（通过 SHAPE_REPRESENTATION_RELATIONSHIP）
    // 这样 SRRWT 引用 SR，符合 AP214 规范，查看器兼容性更好
    int shapeRepId = 0;
    if (!bodyIds.empty()) {
        std::vector<int> items = bodyIds;
        items.push_back(m_axis2Placement3DId);
        // 1. 写 ADVANCED_BREP_SHAPE_REPRESENTATION（存放实体几何）
        int absrId = m_mapper->AllocateNewId();
        WriteEntity(m_builder->BeginEntity(absrId, "ADVANCED_BREP_SHAPE_REPRESENTATION")
            .AddString(compName)
            .AddEntityArray(items)
            .AddEntityRef(m_geometricRepContextId)
            .Build());
        // 2. 写 SHAPE_REPRESENTATION（装配占位 SR，作为子坐标空间）
        shapeRepId = m_mapper->AllocateNewId();
        WriteEntity(m_builder->BeginEntity(shapeRepId, "SHAPE_REPRESENTATION")
            .AddString(compName)
            .AddEntityArray({m_axis2Placement3DId})
            .AddEntityRef(m_geometricRepContextId)
            .Build());
        // 3. 写 SHAPE_REPRESENTATION_RELATIONSHIP：SR → ABSR
        int srrId = m_mapper->AllocateNewId();
        WriteEntity("#" + std::to_string(srrId) +
            "=SHAPE_REPRESENTATION_RELATIONSHIP('','',#" +
            std::to_string(shapeRepId) + ",#" +
            std::to_string(absrId) + ");\n");
        // 4. SDR 指向 SR（而非 ABSR）
        WriteShapeDefinitionRepresentation(productDefShapeId, shapeRepId);
    } else {
        // 纯装配节点：写一个空的 SHAPE_REPRESENTATION
        shapeRepId = m_mapper->AllocateNewId();
        std::string entity = m_builder->BeginEntity(shapeRepId, "SHAPE_REPRESENTATION")
            .AddString(compName)
            .AddEntityArray({m_axis2Placement3DId})
            .AddEntityRef(m_geometricRepContextId)
            .Build();
        WriteEntity(entity);
        WriteShapeDefinitionRepresentation(productDefShapeId, shapeRepId);
    }

    result.productDefId = productDefId;
    result.shapeRepId   = shapeRepId;

    // 递归处理子组件（装配结构）
    Xchg_Size_t numChildren = comp->GetNumChildren();
    for (Xchg_Size_t i = 0; i < numChildren; ++i) {
        Xchg_ComponentInstancePtr inst = comp->GetChild(i);
        if (!inst) continue;
        Xchg_ComponentPtr childComp = inst->GetComponent();
        if (!childComp) continue;

        // 递归写出子组件
        ComponentIds childIds = WriteComponent(childComp);

        // 获取实例变换
        Xchg_transfo trsf = inst->GetTransform();

        // 写出装配关系
        WriteAssemblyLink(shapeRepId, productDefId,
                          childIds.shapeRepId, childIds.productDefId,
                          trsf);
    }

    return result;
}

int XchgToSTEPWriter::WriteBodyAsShapeRepresentation(const Xchg_BodyPtr& body) {
    if (!body) return 0;
    int bodyId = WriteBody(body);
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


