#pragma once

#include <string>
#include <memory>
#include <ostream>
#include <fstream>

// Forward declarations
template<typename T>
class SmartPtr;
class Xchg_MainDoc;
class Xchg_Component;
class Xchg_Body;
class Xchg_Lump;
class Xchg_Shell;
class Xchg_Face;
class Xchg_Loop;
class Xchg_Coedge;
class Xchg_Edge;
class Xchg_Vertex;
class Xchg_Surface;
class Xchg_Curve;
class Xchg_Point;

// 几何类型前向声明
class Xchg_PlaneSurface;
class Xchg_CylindricalSurface;
class Xchg_ConicalSurface;
class Xchg_SphericalSurface;
class Xchg_ToroidalSurface;
class Xchg_NurbsSurface;
class Xchg_Line;
class Xchg_Ellipse;
class Xchg_NurbsCurve;
class Xchg_Polyline;

typedef SmartPtr<Xchg_MainDoc> Xchg_MainDocPtr;
typedef SmartPtr<Xchg_Component> Xchg_ComponentPtr;
typedef SmartPtr<Xchg_Body> Xchg_BodyPtr;
typedef SmartPtr<Xchg_Lump> Xchg_LumpPtr;
typedef SmartPtr<Xchg_Shell> Xchg_ShellPtr;
typedef SmartPtr<Xchg_Face> Xchg_FacePtr;
typedef SmartPtr<Xchg_Loop> Xchg_LoopPtr;
typedef SmartPtr<Xchg_Coedge> Xchg_CoedgePtr;
typedef SmartPtr<Xchg_Edge> Xchg_EdgePtr;
typedef SmartPtr<Xchg_Vertex> Xchg_VertexPtr;
typedef SmartPtr<Xchg_Surface> Xchg_SurfacePtr;
typedef SmartPtr<Xchg_Curve> Xchg_CurvePtr;
typedef SmartPtr<Xchg_Point> Xchg_PointPtr;

// Xchg_Circle 是 Xchg_Ellipse 的别名
typedef Xchg_Ellipse Xchg_Circle;



class STEPEntityBuilder;
class XchgEntityMapper;

/**
 * @brief 长度单位枚举
 */
enum class STEPLengthUnit {
    Millimeter,
    Meter,
    Inch
};

/**
 * @brief Xchg MainDoc 到 STEP 文件的转换器
 *
 * 使用方式：
 * XchgToSTEPWriter writer("output.step");
 * writer.SetLengthUnit(STEPLengthUnit::Millimeter);
 * writer.SetPrecision(1e-6);
 * writer.WriteMainDoc(mainDoc);
 * writer.Done();
 */
class XchgToSTEPWriter {
public:
    // 构造函数
    explicit XchgToSTEPWriter(const std::string& filepath);
    explicit XchgToSTEPWriter(std::ostream& os);
    ~XchgToSTEPWriter();

    // 设置选项
    void SetLengthUnit(STEPLengthUnit unit);
    void SetPrecision(double uncertainty);

    // 写出主文档
    bool WriteMainDoc(const Xchg_MainDocPtr& mainDoc);

    // 完成写出
    bool Done();

private:
    // 文件头部和尾部
    void WriteHeader();
    void WriteFooter();

    // 全局上下文
    void WriteGlobalContext();
    int WriteApplicationContext();
    int WriteGeometricRepresentationContext();
    int WriteGlobalUncertaintyAssignedContext();
    int WriteGlobalUnitAssignedContext();

    // 产品结构
    int WriteProduct(const std::string& name);
    int WriteProductDefinitionFormation(int productId);
    int WriteProductDefinition(int productDefFormationId);
    int WriteProductDefinitionContext(int applicationContextId);
    int WriteProductDefinitionShape(int productDefId);
    int WriteShapeDefinitionRepresentation(int productDefShapeId, int shapeRepId);

    // 装配结构
    void WriteComponent(const Xchg_ComponentPtr& comp);

    // 几何表示
    int WriteBodyAsShapeRepresentation(const Xchg_BodyPtr& body);
    int WriteBody(const Xchg_BodyPtr& body);
    int WriteLump(const Xchg_LumpPtr& lump);
    int WriteShell(const Xchg_ShellPtr& shell, bool isClosed);
    int WriteFace(const Xchg_FacePtr& face, bool faceOrientation);
    int WriteLoop(const Xchg_LoopPtr& loop, bool isOuter);
    int WriteEdgeLoop(const Xchg_LoopPtr& loop);
    int WriteCoedge(const Xchg_CoedgePtr& coedge, bool inLoop);
    int WriteEdge(const Xchg_EdgePtr& edge);
    int WriteVertex(const Xchg_VertexPtr& vertex);

    // 几何元素
    int WriteSurface(const Xchg_SurfacePtr& surface);
    int WriteCurve(const Xchg_CurvePtr& curve);
    int WritePoint(const Xchg_PointPtr& point);
    int WriteDirection(double x, double y, double z);
    int WriteVector(double x, double y, double z, double magnitude);
    int WriteAxis2Placement3D(double ox, double oy, double oz,
                              double zx, double zy, double zz,
                              double xx, double xy, double xz);

    // 具体曲面类型
    int WritePlaneSurface(Xchg_PlaneSurface* plane);
    int WriteCylindricalSurface(Xchg_CylindricalSurface* cylinder);
    int WriteConicalSurface(Xchg_ConicalSurface* cone);
    int WriteSphericalSurface(Xchg_SphericalSurface* sphere);
    int WriteToroidalSurface(Xchg_ToroidalSurface* torus);
    int WriteNurbsSurface(Xchg_NurbsSurface* nurbs);

    // 具体曲线类型
    int WriteLine(Xchg_Line* line);
    int WriteCircle(Xchg_Circle* circle);
    int WriteEllipse(Xchg_Ellipse* ellipse);
    int WriteNurbsCurve(Xchg_NurbsCurve* nurbs);
    int WritePolyline(Xchg_Polyline* polyline);

    // 辅助函数
    void WriteEntity(const std::string& entity);
    std::string GetLengthUnitString() const;
    double GetLengthUnitFactor() const;

private:
    std::unique_ptr<std::ofstream> m_fileStream;
    std::ostream* m_output;
    bool m_ownsStream;

    std::unique_ptr<STEPEntityBuilder> m_builder;
    std::unique_ptr<XchgEntityMapper> m_mapper;

    STEPLengthUnit m_lengthUnit;
    double m_uncertainty;

    // 全局上下文实体 ID
    int m_applicationContextId;
    int m_geometricRepContextId;
    int m_globalUncertaintyId;
    int m_globalUnitId;
    int m_axis2Placement3DId;

    bool m_initialized;
    bool m_done;
};

