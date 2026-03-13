"""
STEP 文件几何对比工具
使用 OCC (via cadquery-ocp) 读取两个 STEP 文件，逐 solid 对比：
  - 包围盒
  - 体积
  - 表面积
  - 面数、边数、顶点数
  - 每个面的面积和曲面类型

用法: python step_compare.py <original.step> <exported.step>
"""
import sys
from OCP.STEPControl import STEPControl_Reader
from OCP.BRepGProp import BRepGProp
from OCP.GProp import GProp_GProps
from OCP.Bnd import Bnd_Box
from OCP.BRepBndLib import BRepBndLib
from OCP.TopExp import TopExp_Explorer
from OCP.TopAbs import (TopAbs_SOLID, TopAbs_FACE, TopAbs_EDGE,
                         TopAbs_VERTEX, TopAbs_SHELL)
from OCP.TopoDS import TopoDS
from OCP.BRep import BRep_Tool
from OCP.GeomAdaptor import GeomAdaptor_Surface
from OCP.GeomAbs import (GeomAbs_Plane, GeomAbs_Cylinder, GeomAbs_Cone,
                          GeomAbs_Sphere, GeomAbs_Torus, GeomAbs_BSplineSurface,
                          GeomAbs_BezierSurface, GeomAbs_SurfaceOfRevolution,
                          GeomAbs_SurfaceOfExtrusion, GeomAbs_OffsetSurface,
                          GeomAbs_OtherSurface)


SURF_TYPE_NAMES = {
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
    GeomAbs_OtherSurface: "Other",
}


def read_step(path):
    reader = STEPControl_Reader()
    status = reader.ReadFile(path)
    if str(status) != "IFSelect_ReturnStatus.IFSelect_RetDone":
        print(f"  [ERROR] Failed to read: {path}, status={status}")
        return None
    reader.TransferRoots()
    return reader.OneShape()


def get_bbox(shape):
    box = Bnd_Box()
    BRepBndLib.Add_s(shape, box)
    return box.Get()  # xmin,ymin,zmin,xmax,ymax,zmax


def get_volume(shape):
    props = GProp_GProps()
    BRepGProp.VolumeProperties_s(shape, props)
    return props.Mass()


def get_area(shape):
    props = GProp_GProps()
    BRepGProp.SurfaceProperties_s(shape, props)
    return props.Mass()


def count_topo(shape, topo_type):
    exp = TopExp_Explorer(shape, topo_type)
    n = 0
    while exp.More():
        n += 1
        exp.Next()
    return n


def get_face_info(shape):
    """返回每个面的 (面积, 曲面类型) 列表，按面积降序排列"""
    faces = []
    exp = TopExp_Explorer(shape, TopAbs_FACE)
    while exp.More():
        face = TopoDS.Face_s(exp.Current())
        area = get_area(face)
        surf = BRep_Tool.Surface_s(face)
        if surf:
            adaptor = GeomAdaptor_Surface(surf)
            stype = SURF_TYPE_NAMES.get(adaptor.GetType(), f"Unknown({adaptor.GetType()})")
        else:
            stype = "(null)"
        faces.append((area, stype))
        exp.Next()
    faces.sort(key=lambda x: -x[0])
    return faces


def get_solids(shape):
    """提取所有 solid"""
    solids = []
    exp = TopExp_Explorer(shape, TopAbs_SOLID)
    while exp.More():
        solids.append(TopoDS.Solid_s(exp.Current()))
        exp.Next()
    return solids


def fmt_bbox(bb):
    return f"({bb[0]:.4f},{bb[1]:.4f},{bb[2]:.4f})-({bb[3]:.4f},{bb[4]:.4f},{bb[5]:.4f})"


def compare_solids(ref_solid, exp_solid, idx):
    """对比两个 solid，返回差异列表"""
    diffs = []
    label = f"Solid[{idx}]"

    # 体积
    rv, ev = get_volume(ref_solid), get_volume(exp_solid)
    rel = abs(rv - ev) / max(abs(rv), 1e-15)
    if rel > 1e-4:
        diffs.append(f"  {label} Volume: ref={rv:.6f} exp={ev:.6f} rel_err={rel:.6e}")

    # 面积
    ra, ea = get_area(ref_solid), get_area(exp_solid)
    rel_a = abs(ra - ea) / max(abs(ra), 1e-15)
    if rel_a > 1e-4:
        diffs.append(f"  {label} Area: ref={ra:.6f} exp={ea:.6f} rel_err={rel_a:.6e}")

    # 包围盒
    rb, eb = get_bbox(ref_solid), get_bbox(exp_solid)
    bbox_diff = max(abs(rb[i] - eb[i]) for i in range(6))
    if bbox_diff > 0.1:
        diffs.append(f"  {label} BBox diff={bbox_diff:.6f}")
        diffs.append(f"    ref: {fmt_bbox(rb)}")
        diffs.append(f"    exp: {fmt_bbox(eb)}")

    # 拓扑计数
    for ttype, tname in [(TopAbs_FACE, "Faces"), (TopAbs_EDGE, "Edges"),
                          (TopAbs_VERTEX, "Vertices")]:
        rc, ec = count_topo(ref_solid, ttype), count_topo(exp_solid, ttype)
        if rc != ec:
            diffs.append(f"  {label} {tname}: ref={rc} exp={ec}")

    # 逐面面积对比（按面积排序后配对）
    rf = get_face_info(ref_solid)
    ef = get_face_info(exp_solid)
    n = min(len(rf), len(ef))
    face_diffs = []
    for i in range(n):
        r_area, r_type = rf[i]
        e_area, e_type = ef[i]
        area_rel = abs(r_area - e_area) / max(abs(r_area), 1e-15)
        if r_type != e_type:
            face_diffs.append(
                f"    Face[{i}] type: ref={r_type} exp={e_type} "
                f"(area ref={r_area:.6f} exp={e_area:.6f})")
        elif area_rel > 1e-3:
            face_diffs.append(
                f"    Face[{i}] ({r_type}) area: ref={r_area:.6f} exp={e_area:.6f} "
                f"rel_err={area_rel:.6e}")
    if face_diffs:
        diffs.append(f"  {label} Face-level differences:")
        diffs.extend(face_diffs)

    return diffs


def compare_files(ref_path, exp_path, verbose=False, brief=False):
    """对比两个 STEP 文件，返回 (passed: bool, summary: str)。
    brief=True 时只返回 solid 级摘要（体积/面积误差），不含 face-level diff。
    可被其他脚本 import 调用。"""
    ref_shape = read_step(ref_path)
    if ref_shape is None:
        return None, "OCC cannot read original"

    exp_shape = read_step(exp_path)
    if exp_shape is None:
        return False, "OCC cannot read export"

    diffs = []

    # solid 数
    rs = count_topo(ref_shape, TopAbs_SOLID)
    es = count_topo(exp_shape, TopAbs_SOLID)
    if rs != es:
        diffs.append(f"solids: ref={rs} exp={es}")

    # 逐 solid 对比
    ref_solids = get_solids(ref_shape)
    exp_solids = get_solids(exp_shape)
    ref_solids.sort(key=lambda s: -abs(get_volume(s)))
    exp_solids.sort(key=lambda s: -abs(get_volume(s)))

    n = min(len(ref_solids), len(exp_solids))
    for i in range(n):
        solid_diffs = compare_solids(ref_solids[i], exp_solids[i], i)
        if brief and solid_diffs:
            # 只保留体积/面积/face数/BBox的摘要行
            diffs.extend(d for d in solid_diffs
                         if "Volume:" in d or "Area:" in d or "Faces:" in d or "BBox" in d)
        elif solid_diffs:
            diffs.extend(solid_diffs)

    passed = len(diffs) == 0
    summary = "; ".join(diffs) if diffs else "OK"

    if verbose:
        # 打印详细信息（和原来 main 一样）
        print(f"Reference: {ref_path}")
        print(f"Exported:  {exp_path}")
        print()
        print("=== Overall Comparison ===")
        for name, shape in [("REF", ref_shape), ("EXP", exp_shape)]:
            v = get_volume(shape)
            a = get_area(shape)
            bb = get_bbox(shape)
            ns = count_topo(shape, TopAbs_SOLID)
            nf = count_topo(shape, TopAbs_FACE)
            ne = count_topo(shape, TopAbs_EDGE)
            print(f"  {name}: solids={ns} faces={nf} edges={ne} "
                  f"vol={v:.4f} area={a:.4f} bbox={fmt_bbox(bb)}")
        print()
        if passed:
            print("=== ALL SOLIDS MATCH ===")
        else:
            print("=== DIFFERENCES FOUND ===")
            for d in diffs:
                print(d)

    return passed, summary


def main():
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <original.step> <exported.step>")
        sys.exit(1)

    ref_path, exp_path = sys.argv[1], sys.argv[2]
    passed, _ = compare_files(ref_path, exp_path, verbose=True)
    sys.exit(0 if passed else 1)


if __name__ == "__main__":
    main()
