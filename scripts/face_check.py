"""
STEP 导出文件无效面分析工具

用法:
  python scripts/face_check.py <exported.step>

功能:
  用 OCC 读取 STEP 文件，检查每个面的有效性，
  按曲面类型和错误类型分类统计无效面，输出详细信息。
"""
import sys

from OCP.STEPControl import STEPControl_Reader
from OCP.BRepCheck import BRepCheck_Analyzer
from OCP.TopExp import TopExp_Explorer
from OCP.TopAbs import TopAbs_FACE, TopAbs_WIRE, TopAbs_EDGE
from OCP.TopoDS import TopoDS
from OCP.BRep import BRep_Tool
from OCP.GeomAdaptor import GeomAdaptor_Surface
from OCP.BRepGProp import BRepGProp
from OCP.GProp import GProp_GProps
from OCP.GeomAbs import (GeomAbs_Plane, GeomAbs_Cylinder, GeomAbs_Cone,
                          GeomAbs_Sphere, GeomAbs_Torus, GeomAbs_BSplineSurface,
                          GeomAbs_BezierSurface, GeomAbs_SurfaceOfRevolution,
                          GeomAbs_SurfaceOfExtrusion, GeomAbs_OffsetSurface)

SURF_NAMES = {
    GeomAbs_Plane: "Plane",
    GeomAbs_Cylinder: "Cylinder",
    GeomAbs_Cone: "Cone",
    GeomAbs_Sphere: "Sphere",
    GeomAbs_Torus: "Torus",
    GeomAbs_BSplineSurface: "BSpline",
    GeomAbs_BezierSurface: "Bezier",
    GeomAbs_SurfaceOfRevolution: "Revolution",
    GeomAbs_SurfaceOfExtrusion: "Extrusion",
    GeomAbs_OffsetSurface: "Offset",
}


def surf_type_name(face):
    surf = BRep_Tool.Surface_s(face)
    if not surf:
        return "null"
    adaptor = GeomAdaptor_Surface(surf)
    return SURF_NAMES.get(adaptor.GetType(), f"Other({adaptor.GetType()})")


def face_area(face):
    props = GProp_GProps()
    BRepGProp.SurfaceProperties_s(face, props)
    return props.Mass()


def count_sub(face, topo_type):
    exp = TopExp_Explorer(face, topo_type)
    n = 0
    while exp.More():
        n += 1
        exp.Next()
    return n


def get_face_errors(face):
    """返回面级别 + 子拓扑级别的 BRepCheck 错误列表"""
    analyzer = BRepCheck_Analyzer(face, False)
    errors = set()

    # 面自身
    result = analyzer.Result(face)
    if result:
        for s in result.Status():
            name = str(s).split(".")[-1]
            if name != "BRepCheck_NoError":
                errors.add(name)

    # wire
    wexp = TopExp_Explorer(face, TopAbs_WIRE)
    while wexp.More():
        wr = analyzer.Result(wexp.Current())
        if wr:
            for s in wr.Status():
                name = str(s).split(".")[-1]
                if name != "BRepCheck_NoError":
                    errors.add(name)
        wexp.Next()

    # edge
    eexp = TopExp_Explorer(face, TopAbs_EDGE)
    while eexp.More():
        er = analyzer.Result(eexp.Current())
        if er:
            for s in er.Status():
                name = str(s).split(".")[-1]
                if name != "BRepCheck_NoError":
                    errors.add(name)
        eexp.Next()

    return sorted(errors)


def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <step_file>")
        sys.exit(1)

    path = sys.argv[1]
    reader = STEPControl_Reader()
    status = reader.ReadFile(path)
    if str(status) != "IFSelect_ReturnStatus.IFSelect_RetDone":
        print(f"[ERROR] Failed to read: {path}")
        sys.exit(1)

    reader.TransferRoots()
    shape = reader.OneShape()

    # 遍历所有面
    exp = TopExp_Explorer(shape, TopAbs_FACE)
    idx = 0
    total = 0
    invalid_faces = []
    type_counts = {}   # surf_type -> count
    error_counts = {}  # error_name -> count

    while exp.More():
        total += 1
        face = TopoDS.Face_s(exp.Current())
        analyzer = BRepCheck_Analyzer(face)

        if not analyzer.IsValid():
            stype = surf_type_name(face)
            area = face_area(face)
            nw = count_sub(face, TopAbs_WIRE)
            ne = count_sub(face, TopAbs_EDGE)
            errors = get_face_errors(face)

            invalid_faces.append((idx, stype, area, nw, ne, errors))
            type_counts[stype] = type_counts.get(stype, 0) + 1
            for e in errors:
                error_counts[e] = error_counts.get(e, 0) + 1

        idx += 1
        exp.Next()

    # 输出
    n_invalid = len(invalid_faces)
    print(f"File: {path}")
    print(f"Total faces: {total}")
    print(f"Invalid faces: {n_invalid}")

    if n_invalid == 0:
        print("All faces valid.")
        sys.exit(0)

    print()
    print("=== By surface type ===")
    for t, c in sorted(type_counts.items(), key=lambda x: -x[1]):
        print(f"  {t}: {c}")

    print()
    print("=== By error type ===")
    for e, c in sorted(error_counts.items(), key=lambda x: -x[1]):
        print(f"  {e}: {c}")

    print()
    print("=== Invalid face details ===")
    for i, stype, area, nw, ne, errors in invalid_faces:
        err_str = ", ".join(errors)
        print(f"  Face[{i}]: {stype}  area={area:.4f}  wires={nw} edges={ne}  errors=[{err_str}]")

    sys.exit(1)


if __name__ == "__main__":
    main()
