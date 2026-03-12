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
        m_mapper->SetMapping(surface, surfaceId);  // 记录映射
    }

    return surfaceId;
}

int XchgToSTEPWriter::WritePlaneSurface(Xchg_PlaneSurface* plane) {
    if (!plane) return 0;

    // 获取平面参数
    Xchg_pnt origin = plane->GetOrigin();
    Xchg_dir normal = plane->GetNormal();

    // 直接从 Xchg_PlaneSurface 读取 X 方向（即 UDirection），保留原始方向信息
    Xchg_dir xAxis = plane->GetXDirection();

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

    int degreeU = nurbs->GetUDegree();
    int degreeV = nurbs->GetVDegree();
    int numU = nurbs->GetNumPointsU();
    int numV = nurbs->GetNumPointsV();
    bool isRational = nurbs->IsRational();

    bool uClosed = nurbs->IsUClosed();
    bool vClosed = nurbs->IsVClosed();

    // 写出控制点（按 u-major 顺序存储，STEP 要求外层 u、内层 v）
    std::vector<int> cpIds;
    for (int u = 0; u < numU; ++u) {
        for (int v = 0; v < numV; ++v) {
            const Xchg_pnt& pt = nurbs->Point(u, v);
            int ptId = m_mapper->AllocateNewId();
            std::string entity = m_builder->BeginEntity(ptId, "CARTESIAN_POINT")
                .AddString("")
                .AddRealArray({pt.x(), pt.y(), pt.z()})
                .Build();
            WriteEntity(entity);
            cpIds.push_back(ptId);
        }
    }

    // 获取节点向量
    std::vector<double> knotsU, knotsV;
    std::vector<int> multsU, multsV;
    for (int i = 0; i < nurbs->GetNumKnotsU(); ++i) {
        knotsU.push_back(nurbs->GetUKnotValue(i));
        multsU.push_back(nurbs->GetUKnotMultiplicity(i));
    }
    for (int i = 0; i < nurbs->GetNumKnotsV(); ++i) {
        knotsV.push_back(nurbs->GetVKnotValue(i));
        multsV.push_back(nurbs->GetVKnotMultiplicity(i));
    }

    int nurbsId = m_mapper->AllocateNewId();

    // 辅助：写出控制点二维数组（STEP 要求外层 u、内层 v）
    auto writeCPs2D = [&]() {
        *m_output << "((";
        for (int u = 0; u < numU; ++u) {
            if (u > 0) *m_output << "),(";
            for (int v = 0; v < numV; ++v) {
                if (v > 0) *m_output << ",";
                *m_output << "#" << cpIds[u * numV + v];
            }
        }
        *m_output << "))";
    };
    auto writeIntArr = [&](const std::vector<int>& a) {
        *m_output << "(";
        for (size_t i = 0; i < a.size(); ++i) { if (i) *m_output << ","; *m_output << a[i]; }
        *m_output << ")";
    };
    auto writeRealArr = [&](const std::vector<double>& a) {
        *m_output << "(";
        for (size_t i = 0; i < a.size(); ++i) { if (i) *m_output << ","; *m_output << a[i]; }
        *m_output << ")";
    };

    if (isRational) {
        // 复合实体形式（子类型按字母序）
        *m_output << "#" << nurbsId << "=(";
        *m_output << "BOUNDED_SURFACE()";
        *m_output << "B_SPLINE_SURFACE(" << degreeU << "," << degreeV << ",";
        writeCPs2D();
        *m_output << ",.UNSPECIFIED.,"
                  << (uClosed ? ".T." : ".F.") << ","
                  << (vClosed ? ".T." : ".F.") << ",.F.)";
        *m_output << "B_SPLINE_SURFACE_WITH_KNOTS(";
        writeIntArr(multsU); *m_output << ","; writeIntArr(multsV); *m_output << ",";
        writeRealArr(knotsU); *m_output << ","; writeRealArr(knotsV);
        *m_output << ",.UNSPECIFIED.)";
        *m_output << "GEOMETRIC_REPRESENTATION_ITEM()";
        *m_output << "RATIONAL_B_SPLINE_SURFACE((";
        for (int u = 0; u < numU; ++u) {
            if (u > 0) *m_output << ",(";
            else *m_output << "(";
            for (int v = 0; v < numV; ++v) {
                if (v > 0) *m_output << ",";
                *m_output << nurbs->GetWeight(u, v);
            }
            *m_output << ")";
        }
        *m_output << "))";
        *m_output << "REPRESENTATION_ITEM('')";
        *m_output << "SURFACE()";
        *m_output << ");\n";
    } else {
        // 非 rational：简单实体
        *m_output << "#" << nurbsId << "=B_SPLINE_SURFACE_WITH_KNOTS('',";
        *m_output << degreeU << "," << degreeV << ",";
        writeCPs2D();
        *m_output << ",.UNSPECIFIED.,"
                  << (uClosed ? ".T." : ".F.") << ","
                  << (vClosed ? ".T." : ".F.") << ",.F.,";
        writeIntArr(multsU); *m_output << ","; writeIntArr(multsV); *m_output << ",";
        writeRealArr(knotsU); *m_output << ","; writeRealArr(knotsV);
        *m_output << ",.UNSPECIFIED.);\n";
    }

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
        m_mapper->SetMapping(curve, curveId);  // 记录映射
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

    int degree = nurbs->GetDegree();
    int numPts = nurbs->GetNumPoints();
    bool isRational = nurbs->IsRational();

    // 写出控制点
    std::vector<int> cpIds;
    for (int i = 0; i < numPts; ++i) {
        const Xchg_pnt& pt = nurbs->Point(i);
        int ptId = m_mapper->AllocateNewId();
        std::string entity = m_builder->BeginEntity(ptId, "CARTESIAN_POINT")
            .AddString("")
            .AddRealArray({pt.x(), pt.y(), pt.z()})
            .Build();
        WriteEntity(entity);
        cpIds.push_back(ptId);
    }

    // 获取节点向量和重数
    std::vector<double> knots;
    std::vector<int> mults;
    for (int i = 0; i < nurbs->GetNumKnots(); ++i) {
        knots.push_back(nurbs->GetKnotValue(i));
        mults.push_back(nurbs->GetKnotMultiplicity(i));
    }

    int nurbsId = m_mapper->AllocateNewId();

    auto writeCPs = [&]() {
        *m_output << "(";
        for (size_t i = 0; i < cpIds.size(); ++i) {
            if (i) *m_output << ",";
            *m_output << "#" << cpIds[i];
        }
        *m_output << ")";
    };
    auto writeIntArr = [&](const std::vector<int>& a) {
        *m_output << "(";
        for (size_t i = 0; i < a.size(); ++i) { if (i) *m_output << ","; *m_output << a[i]; }
        *m_output << ")";
    };
    auto writeRealArr = [&](const std::vector<double>& a) {
        *m_output << "(";
        for (size_t i = 0; i < a.size(); ++i) { if (i) *m_output << ","; *m_output << a[i]; }
        *m_output << ")";
    };

    if (isRational) {
        // 复合实体形式（子类型按字母序）
        *m_output << "#" << nurbsId << "=(";
        *m_output << "BOUNDED_CURVE()";
        *m_output << "B_SPLINE_CURVE(" << degree << ",";
        writeCPs();
        *m_output << ",.UNSPECIFIED.,.F.,.F.)";
        *m_output << "B_SPLINE_CURVE_WITH_KNOTS(";
        writeIntArr(mults); *m_output << ",";
        writeRealArr(knots);
        *m_output << ",.UNSPECIFIED.)";
        *m_output << "CURVE()";
        *m_output << "GEOMETRIC_REPRESENTATION_ITEM()";
        *m_output << "RATIONAL_B_SPLINE_CURVE(";
        *m_output << "(";
        for (int i = 0; i < numPts; ++i) {
            if (i) *m_output << ",";
            *m_output << nurbs->GetWeight(i);
        }
        *m_output << "))";
        *m_output << "REPRESENTATION_ITEM('')";
        *m_output << ");\n";
    } else {
        // 非 rational：简单实体
        *m_output << "#" << nurbsId << "=B_SPLINE_CURVE_WITH_KNOTS('',";
        *m_output << degree << ",";
        writeCPs();
        *m_output << ",.UNSPECIFIED.,.F.,.F.,";
        writeIntArr(mults); *m_output << ",";
        writeRealArr(knots);
        *m_output << ",.UNSPECIFIED.);\n";
    }

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

