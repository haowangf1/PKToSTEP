#include "../include/xchg_to_step_writer.hpp"
#include "../include/step_entity_builder.hpp"
#include "../include/xchg_entity_mapper.hpp"

#include "geom/surface/xchg_surface.hpp"
#include "geom/surface/xchg_planesurface.hpp"
#include "geom/surface/xchg_cylindricalsurface.hpp"
#include "geom/surface/xchg_conicalsurface.hpp"
#include "geom/surface/xchg_sphericalsurface.hpp"
#include "geom/surface/xchg_toroidalsurface.hpp"
#include "geom/surface/xchg_nurbssurface.hpp"

#include "geom/curve/xchg_curve.hpp"
#include "geom/curve/xchg_line.hpp"
#include "geom/curve/xchg_ellipse.hpp"
#include "geom/curve/xchg_nurbscurve.hpp"
#include "geom/curve/xchg_polyline.hpp"

#include "geom/xchg_point.hpp"
#include "base/xchg_dir.hpp"

#include <iostream>


int XchgToSTEPWriter::WriteSurface(const Xchg_SurfacePtr& surface) {
    if (!surface) {
        std::cerr << "[Warning] Surface is null" << std::endl;
        return 0;
    }

    // 检查是否已经写出
    if (m_mapper->HasMapping(surface)) {
        return m_mapper->GetMapping(surface);
    }

    // 根据曲面类型分发
    XCHG_TYPE_ENUM surfaceType = surface->GetType();

    int surfaceId = 0;

    switch (surfaceType) {
        case XCHG_TYPE_PLANE: {
            Xchg_PlaneSurface* plane = dynamic_cast<Xchg_PlaneSurface*>(surface.get());
            if (plane) {
                surfaceId = WritePlaneSurface(plane);
            }
            break;
        }
        case XCHG_TYPE_CYLINDRICAL_SURFACE: {
            Xchg_CylindricalSurface* cylinder = dynamic_cast<Xchg_CylindricalSurface*>(surface.get());
            if (cylinder) {
                surfaceId = WriteCylindricalSurface(cylinder);
            }
            break;
        }
        case XCHG_TYPE_CONICAL_SURFACE: {
            Xchg_ConicalSurface* cone = dynamic_cast<Xchg_ConicalSurface*>(surface.get());
            if (cone) {
                surfaceId = WriteConicalSurface(cone);
            }
            break;
        }
        case XCHG_TYPE_SPHERICAL_SURFACE: {
            Xchg_SphericalSurface* sphere = dynamic_cast<Xchg_SphericalSurface*>(surface.get());
            if (sphere) {
                surfaceId = WriteSphericalSurface(sphere);
            }
            break;
        }
        case XCHG_TYPE_TOROIDAL_SURFACE: {
            Xchg_ToroidalSurface* torus = dynamic_cast<Xchg_ToroidalSurface*>(surface.get());
            if (torus) {
                surfaceId = WriteToroidalSurface(torus);
            }
            break;
        }
        case XCHG_TYPE_NURBS_SURFACE: {
            Xchg_NurbsSurface* nurbs = dynamic_cast<Xchg_NurbsSurface*>(surface.get());
            if (nurbs) {
                surfaceId = WriteNurbsSurface(nurbs);
            }
            break;
        }
        default:
            std::cerr << "[Warning] Unsupported surface type: " << surfaceType << std::endl;
            break;
    }

    if (surfaceId > 0) {
        m_mapper->GetOrAllocate(surface);  // 记录映射
    }

    return surfaceId;
}

int XchgToSTEPWriter::WritePlaneSurface(Xchg_PlaneSurface* plane) {
    if (!plane) return 0;

    // 获取平面参数
    Xchg_pnt origin = plane->GetOrigin();
    Xchg_dir normal = plane->GetNormal();

    // 计算 X 轴（需要一个与法向量垂直的向量）
    Xchg_dir xAxis;
    if (std::abs(normal.x()) < 0.9) {
        xAxis = Xchg_dir(1, 0, 0);
    } else {
        xAxis = Xchg_dir(0, 1, 0);
    }
    // 使用叉积计算正交的 X 轴
    Xchg_dir yAxis = normal ^ xAxis;
    yAxis.normalize();
    xAxis = yAxis ^ normal;
    xAxis.normalize();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(
        origin.x(), origin.y(), origin.z(),
        normal.x(), normal.y(), normal.z(),
        xAxis.x(), xAxis.y(), xAxis.z()
    );

    // 写出 PLANE
    int planeId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(planeId, "PLANE")
        .AddString("")
        .AddEntityRef(axisId)
        .Build();
    WriteEntity(entity);

    return planeId;
}

int XchgToSTEPWriter::WriteCylindricalSurface(Xchg_CylindricalSurface* cylinder) {
    if (!cylinder) return 0;

    // 获取圆柱参数
    Xchg_pnt origin = cylinder->GetOrigin();
    Xchg_dir zAxis = cylinder->GetZDirection();  // 轴向
    Xchg_dir xAxis = cylinder->GetXDirection();  // X 方向
    double radius = cylinder->GetRadius();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(
        origin.x(), origin.y(), origin.z(),
        zAxis.x(), zAxis.y(), zAxis.z(),
        xAxis.x(), xAxis.y(), xAxis.z()
    );

    // 写出 CYLINDRICAL_SURFACE
    int cylinderId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(cylinderId, "CYLINDRICAL_SURFACE")
        .AddString("")
        .AddEntityRef(axisId)
        .AddReal(radius)
        .Build();
    WriteEntity(entity);

    return cylinderId;
}

int XchgToSTEPWriter::WriteConicalSurface(Xchg_ConicalSurface* cone) {
    if (!cone) return 0;

    // 获取圆锥参数
    Xchg_pnt origin = cone->GetOrigin();
    Xchg_dir zAxis = cone->GetZDirection();
    Xchg_dir xAxis = cone->GetXDirection();
    double radius = cone->GetRadius();
    double semiAngle = cone->GetSemiAngle();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(
        origin.x(), origin.y(), origin.z(),
        zAxis.x(), zAxis.y(), zAxis.z(),
        xAxis.x(), xAxis.y(), xAxis.z()
    );

    // 写出 CONICAL_SURFACE
    int coneId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(coneId, "CONICAL_SURFACE")
        .AddString("")
        .AddEntityRef(axisId)
        .AddReal(radius)
        .AddReal(semiAngle)
        .Build();
    WriteEntity(entity);

    return coneId;
}

int XchgToSTEPWriter::WriteSphericalSurface(Xchg_SphericalSurface* sphere) {
    if (!sphere) return 0;

    // 获取球面参数
    Xchg_pnt origin = sphere->GetOrigin();
    Xchg_dir zAxis = sphere->GetZDirection();
    Xchg_dir xAxis = sphere->GetXDirection();
    double radius = sphere->GetRadius();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(
        origin.x(), origin.y(), origin.z(),
        zAxis.x(), zAxis.y(), zAxis.z(),
        xAxis.x(), xAxis.y(), xAxis.z()
    );

    // 写出 SPHERICAL_SURFACE
    int sphereId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(sphereId, "SPHERICAL_SURFACE")
        .AddString("")
        .AddEntityRef(axisId)
        .AddReal(radius)
        .Build();
    WriteEntity(entity);

    return sphereId;
}

int XchgToSTEPWriter::WriteToroidalSurface(Xchg_ToroidalSurface* torus) {
    if (!torus) return 0;

    // 获取环面参数
    Xchg_pnt origin = torus->GetOrigin();
    Xchg_dir zAxis = torus->GetZDirection();
    Xchg_dir xAxis = torus->GetXDirection();
    double majorRadius = torus->GetMajorRadius();
    double minorRadius = torus->GetMinorRadius();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(
        origin.x(), origin.y(), origin.z(),
        zAxis.x(), zAxis.y(), zAxis.z(),
        xAxis.x(), xAxis.y(), xAxis.z()
    );

    // 写出 TOROIDAL_SURFACE
    int torusId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(torusId, "TOROIDAL_SURFACE")
        .AddString("")
        .AddEntityRef(axisId)
        .AddReal(majorRadius)
        .AddReal(minorRadius)
        .Build();
    WriteEntity(entity);

    return torusId;
}

int XchgToSTEPWriter::WriteNurbsSurface(Xchg_NurbsSurface* nurbs) {
    if (!nurbs) return 0;

    // 获取 NURBS 参数
    int degreeU = nurbs->GetUDegree();
    int degreeV = nurbs->GetVDegree();

    // 获取控制点数量
    int numControlPointsU = nurbs->GetNumPointsU();
    int numControlPointsV = nurbs->GetNumPointsV();

    // 写出控制点
    std::vector<int> controlPointIds;
    for (int v = 0; v < numControlPointsV; ++v) {
        for (int u = 0; u < numControlPointsU; ++u) {
            const Xchg_pnt& pt = nurbs->Point(u, v);
            int ptId = m_mapper->AllocateNewId();
            std::string entity = m_builder->BeginEntity(ptId, "CARTESIAN_POINT")
                .AddString("")
                .AddRealArray({pt.x(), pt.y(), pt.z()})
                .Build();
            WriteEntity(entity);
            controlPointIds.push_back(ptId);
        }
    }

    // 获取节点向量
    std::vector<double> knotsU, knotsV;
    std::vector<int> multsU, multsV;

    int numKnotsU = nurbs->GetNumKnotsU();
    int numKnotsV = nurbs->GetNumKnotsV();

    for (int i = 0; i < numKnotsU; ++i) {
        knotsU.push_back(nurbs->GetUKnotValue(i));
        multsU.push_back(nurbs->GetUKnotMultiplicity(i));
    }

    for (int i = 0; i < numKnotsV; ++i) {
        knotsV.push_back(nurbs->GetVKnotValue(i));
        multsV.push_back(nurbs->GetVKnotMultiplicity(i));
    }

    // 获取权重
    std::vector<double> weights;
    for (int v = 0; v < numControlPointsV; ++v) {
        for (int u = 0; u < numControlPointsU; ++u) {
            weights.push_back(nurbs->GetWeight(u, v));
        }
    }

    // 写出 B_SPLINE_SURFACE_WITH_KNOTS
    int nurbsId = m_mapper->AllocateNewId();

    // 这是一个复杂实体，需要特殊处理
    *m_output << "#" << nurbsId << "=B_SPLINE_SURFACE_WITH_KNOTS('',";
    *m_output << degreeU << "," << degreeV << ",";

    // 控制点（二维数组）
    *m_output << "((";
    for (int v = 0; v < numControlPointsV; ++v) {
        if (v > 0) *m_output << "),(";
        for (int u = 0; u < numControlPointsU; ++u) {
            if (u > 0) *m_output << ",";
            *m_output << "#" << controlPointIds[v * numControlPointsU + u];
        }
    }
    *m_output << ")),.UNSPECIFIED.,.F.,.F.,.F.,";

    // 节点向量 U 的重数
    *m_output << "(";
    for (size_t i = 0; i < multsU.size(); ++i) {
        if (i > 0) *m_output << ",";
        *m_output << multsU[i];
    }
    *m_output << "),(";

    // 节点向量 V 的重数
    for (size_t i = 0; i < multsV.size(); ++i) {
        if (i > 0) *m_output << ",";
        *m_output << multsV[i];
    }
    *m_output << "),(";

    // 节点向量 U 的值
    for (size_t i = 0; i < knotsU.size(); ++i) {
        if (i > 0) *m_output << ",";
        *m_output << knotsU[i];
    }
    *m_output << "),(";

    // 节点向量 V 的值
    for (size_t i = 0; i < knotsV.size(); ++i) {
        if (i > 0) *m_output << ",";
        *m_output << knotsV[i];
    }
    *m_output << "),.UNSPECIFIED.);\n";

    return nurbsId;
}

int XchgToSTEPWriter::WriteCurve(const Xchg_CurvePtr& curve) {
    if (!curve) {
        std::cerr << "[Warning] Curve is null" << std::endl;
        return 0;
    }

    // 检查是否已经写出
    if (m_mapper->HasMapping(curve)) {
        return m_mapper->GetMapping(curve);
    }

    // 根据曲线类型分发
    XCHG_TYPE_ENUM curveType = curve->GetType();

    int curveId = 0;

    switch (curveType) {
        case XCHG_TYPE_LINE: {
            Xchg_Line* line = dynamic_cast<Xchg_Line*>(curve.get());
            if (line) {
                curveId = WriteLine(line);
            }
            break;
        }
        case XCHG_TYPE_CIRCLE: {
            Xchg_Circle* circle = dynamic_cast<Xchg_Circle*>(curve.get());
            if (circle) {
                curveId = WriteCircle(circle);
            }
            break;
        }
        case XCHG_TYPE_ELLIPSE: {
            Xchg_Ellipse* ellipse = dynamic_cast<Xchg_Ellipse*>(curve.get());
            if (ellipse) {
                curveId = WriteEllipse(ellipse);
            }
            break;
        }
        case XCHG_TYPE_NURBS_CURVE: {
            Xchg_NurbsCurve* nurbs = dynamic_cast<Xchg_NurbsCurve*>(curve.get());
            if (nurbs) {
                curveId = WriteNurbsCurve(nurbs);
            }
            break;
        }
        case XCHG_TYPE_POLYLINE: {
            Xchg_Polyline* polyline = dynamic_cast<Xchg_Polyline*>(curve.get());
            if (polyline) {
                curveId = WritePolyline(polyline);
            }
            break;
        }
        default:
            std::cerr << "[Warning] Unsupported curve type: " << curveType << std::endl;
            break;
    }

    if (curveId > 0) {
        m_mapper->GetOrAllocate(curve);  // 记录映射
    }

    return curveId;
}

int XchgToSTEPWriter::WriteLine(Xchg_Line* line) {
    if (!line) return 0;

    // 获取直线参数
    Xchg_pnt origin = line->GetOrigin();
    Xchg_dir direction = line->GetDirection();

    // 写出起点
    int originId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(originId, "CARTESIAN_POINT")
        .AddString("")
        .AddRealArray({origin.x(), origin.y(), origin.z()})
        .Build();
    WriteEntity(entity);

    // 写出方向向量
    int vectorId = WriteVector(direction.x(), direction.y(), direction.z(), 1.0);

    // 写出 LINE
    int lineId = m_mapper->AllocateNewId();
    entity = m_builder->BeginEntity(lineId, "LINE")
        .AddString("")
        .AddEntityRef(originId)
        .AddEntityRef(vectorId)
        .Build();
    WriteEntity(entity);

    return lineId;
}

int XchgToSTEPWriter::WriteCircle(Xchg_Circle* circle) {
    // Circle 实际上是 Xchg_Ellipse，当 MajorRadius == MinorRadius 时
    Xchg_Ellipse* ellipse = dynamic_cast<Xchg_Ellipse*>(circle);
    if (!ellipse) return 0;

    // 获取圆参数
    Xchg_pnt center = ellipse->GetCenterPoint();
    Xchg_dir normal = ellipse->GetNormalDirection();
    Xchg_dir xAxis = ellipse->GetXDirection();
    double radius = ellipse->GetMajorRadius();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(
        center.x(), center.y(), center.z(),
        normal.x(), normal.y(), normal.z(),
        xAxis.x(), xAxis.y(), xAxis.z()
    );

    // 写出 CIRCLE
    int circleId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(circleId, "CIRCLE")
        .AddString("")
        .AddEntityRef(axisId)
        .AddReal(radius)
        .Build();
    WriteEntity(entity);

    return circleId;
}

int XchgToSTEPWriter::WriteEllipse(Xchg_Ellipse* ellipse) {
    if (!ellipse) return 0;

    // 获取椭圆参数
    Xchg_pnt center = ellipse->GetCenterPoint();
    Xchg_dir normal = ellipse->GetNormalDirection();
    Xchg_dir xAxis = ellipse->GetXDirection();
    double majorRadius = ellipse->GetMajorRadius();
    double minorRadius = ellipse->GetMinorRadius();

    // 写出 AXIS2_PLACEMENT_3D
    int axisId = WriteAxis2Placement3D(
        center.x(), center.y(), center.z(),
        normal.x(), normal.y(), normal.z(),
        xAxis.x(), xAxis.y(), xAxis.z()
    );

    // 写出 ELLIPSE
    int ellipseId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(ellipseId, "ELLIPSE")
        .AddString("")
        .AddEntityRef(axisId)
        .AddReal(majorRadius)
        .AddReal(minorRadius)
        .Build();
    WriteEntity(entity);

    return ellipseId;
}

int XchgToSTEPWriter::WriteNurbsCurve(Xchg_NurbsCurve* nurbs) {
    if (!nurbs) return 0;

    // 获取 NURBS 参数
    int degree = nurbs->GetDegree();
    int numControlPoints = nurbs->GetNumPoints();

    // 写出控制点
    std::vector<int> controlPointIds;
    for (int i = 0; i < numControlPoints; ++i) {
        const Xchg_pnt& pt = nurbs->Point(i);
        int ptId = m_mapper->AllocateNewId();
        std::string entity = m_builder->BeginEntity(ptId, "CARTESIAN_POINT")
            .AddString("")
            .AddRealArray({pt.x(), pt.y(), pt.z()})
            .Build();
        WriteEntity(entity);
        controlPointIds.push_back(ptId);
    }

    // 获取节点向量和重数
    std::vector<double> knots;
    std::vector<int> mults;

    int numKnots = nurbs->GetNumKnots();
    for (int i = 0; i < numKnots; ++i) {
        knots.push_back(nurbs->GetKnotValue(i));
        mults.push_back(nurbs->GetKnotMultiplicity(i));
    }

    // 写出 B_SPLINE_CURVE_WITH_KNOTS
    int nurbsId = m_mapper->AllocateNewId();

    *m_output << "#" << nurbsId << "=B_SPLINE_CURVE_WITH_KNOTS('',";
    *m_output << degree << ",(";
    for (size_t i = 0; i < controlPointIds.size(); ++i) {
        if (i > 0) *m_output << ",";
        *m_output << "#" << controlPointIds[i];
    }
    *m_output << "),.UNSPECIFIED.,.F.,.F.,(";
    for (size_t i = 0; i < mults.size(); ++i) {
        if (i > 0) *m_output << ",";
        *m_output << mults[i];
    }
    *m_output << "),(";
    for (size_t i = 0; i < knots.size(); ++i) {
        if (i > 0) *m_output << ",";
        *m_output << knots[i];
    }
    *m_output << "),.UNSPECIFIED.);\n";

    return nurbsId;
}

int XchgToSTEPWriter::WritePolyline(Xchg_Polyline* polyline) {
    if (!polyline) return 0;

    // 获取点数
    int numPoints = polyline->GetNumPoints();

    // 写出所有点
    std::vector<int> pointIds;
    for (int i = 0; i < numPoints; ++i) {
        const Xchg_pnt& pt = polyline->Point(i);
        int ptId = m_mapper->AllocateNewId();
        std::string entity = m_builder->BeginEntity(ptId, "CARTESIAN_POINT")
            .AddString("")
            .AddRealArray({pt.x(), pt.y(), pt.z()})
            .Build();
        WriteEntity(entity);
        pointIds.push_back(ptId);
    }

    // 写出 POLYLINE
    int polylineId = m_mapper->AllocateNewId();
    std::string entity = m_builder->BeginEntity(polylineId, "POLYLINE")
        .AddString("")
        .AddEntityArray(pointIds)
        .Build();
    WriteEntity(entity);

    return polylineId;
}

