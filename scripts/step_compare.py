"""
STEP 文件几何对比工具 - 子进程版
每次 OCC 操作在独立子进程中执行，主进程内存不积累。
"""
import sys
import os
import json
import subprocess

# ──────────────────────────────────────────────
# worker 模式：被子进程调用，执行实际 OCC 操作
# ──────────────────────────────────────────────
def _worker_read_and_compare(ref_path, exp_path, brief):
    """在子进程中执行，结果以 JSON 打印到 stdout"""
    from OCP.STEPControl import STEPControl_Reader
    from OCP.BRepGProp import BRepGProp
    from OCP.GProp import GProp_GProps
    from OCP.Bnd import Bnd_Box
    from OCP.BRepBndLib import BRepBndLib
    from OCP.TopExp import TopExp_Explorer
    from OCP.TopAbs import (TopAbs_SOLID, TopAbs_FACE, TopAbs_EDGE, TopAbs_VERTEX)
    from OCP.TopoDS import TopoDS
    from OCP.BRep import BRep_Tool
    from OCP.GeomAdaptor import GeomAdaptor_Surface
    from OCP.GeomAbs import (
        GeomAbs_Plane, GeomAbs_Cylinder, GeomAbs_Cone, GeomAbs_Sphere,
        GeomAbs_Torus, GeomAbs_BSplineSurface, GeomAbs_BezierSurface,
        GeomAbs_SurfaceOfRevolution, GeomAbs_SurfaceOfExtrusion,
        GeomAbs_OffsetSurface, GeomAbs_OtherSurface)
    from OCP.BRepCheck import BRepCheck_Analyzer

    SURF_TYPE_NAMES = {
        GeomAbs_Plane: "Plane", GeomAbs_Cylinder: "Cylinder",
        GeomAbs_Cone: "Cone", GeomAbs_Sphere: "Sphere",
        GeomAbs_Torus: "Torus", GeomAbs_BSplineSurface: "BSpline",
        GeomAbs_BezierSurface: "Bezier", GeomAbs_SurfaceOfRevolution: "Revolution",
        GeomAbs_SurfaceOfExtrusion: "Extrusion", GeomAbs_OffsetSurface: "Offset",
        GeomAbs_OtherSurface: "Other",
    }

    def _read(path):
        r = STEPControl_Reader()
        st = r.ReadFile(path)
        if str(st) != "IFSelect_ReturnStatus.IFSelect_RetDone":
            return None
        r.TransferRoots()
        shape = r.OneShape()
        r.WS().Model().Clear()
        return shape

    def _volume(s):
        p = GProp_GProps(); BRepGProp.VolumeProperties_s(s, p); return p.Mass()

    def _area(s):
        p = GProp_GProps(); BRepGProp.SurfaceProperties_s(s, p); return p.Mass()

    def _bbox(s):
        b = Bnd_Box(); BRepBndLib.Add_s(s, b); return list(b.Get())

    def _count(s, t):
        e = TopExp_Explorer(s, t); n = 0
        while e.More(): n += 1; e.Next()
        e.Clear(); return n

    def _solids(s):
        res = []; e = TopExp_Explorer(s, TopAbs_SOLID)
        while e.More(): res.append(TopoDS.Solid_s(e.Current())); e.Next()
        e.Clear(); return res

    def _face_info(s):
        faces = []; e = TopExp_Explorer(s, TopAbs_FACE)
        while e.More():
            face = TopoDS.Face_s(e.Current())
            area = _area(face)
            surf = BRep_Tool.Surface_s(face)
            if surf:
                ad = GeomAdaptor_Surface(surf)
                stype = SURF_TYPE_NAMES.get(ad.GetType(), "Other")
                del ad
            else:
                stype = "(null)"
            faces.append((area, stype))
            e.Next()
        e.Clear()
        faces.sort(key=lambda x: -x[0])
        return faces

    def _validity(s):
        """返回 (valid, msg)"""
        sc = _count(s, TopAbs_SOLID)
        if sc == 0:
            return False, "no solids in export"
        inv = 0; tot = 0
        e = TopExp_Explorer(s, TopAbs_FACE)
        while e.More():
            face = TopoDS.Face_s(e.Current()); tot += 1
            a = BRepCheck_Analyzer(face)
            if not a.IsValid(): inv += 1
            del a; e.Next()
        e.Clear()
        if inv > 0:
            return False, f"{inv}/{tot} invalid faces"
        return True, "OK"

    # --- 读取 ---
    ref = _read(ref_path)
    if ref is None:
        print(json.dumps({"status": "skip", "msg": "OCC cannot read original"}))
        return

    exp = _read(exp_path)
    if exp is None:
        print(json.dumps({"status": "fail", "msg": "OCC cannot read export"}))
        return

    # --- 有效性 ---
    valid, vmsg = _validity(exp)

    # --- 对比 ---
    diffs = []
    rs = _count(ref, TopAbs_SOLID); es = _count(exp, TopAbs_SOLID)
    if rs != es:
        diffs.append(f"solids: ref={rs} exp={es}")

    ref_solids = sorted(_solids(ref), key=lambda s: -abs(_volume(s)))
    exp_solids = sorted(_solids(exp), key=lambda s: -abs(_volume(s)))

    for i in range(min(len(ref_solids), len(exp_solids))):
        rs2, es2 = ref_solids[i], exp_solids[i]
        rv, ev = _volume(rs2), _volume(es2)
        rel = abs(rv - ev) / max(abs(rv), 1e-15)
        if rel > 1e-4:
            diffs.append(f"Solid[{i}] Volume: ref={rv:.6f} exp={ev:.6f} rel_err={rel:.6e}")
        ra, ea = _area(rs2), _area(es2)
        rel_a = abs(ra - ea) / max(abs(ra), 1e-15)
        if rel_a > 1e-4:
            diffs.append(f"Solid[{i}] Area: ref={ra:.6f} exp={ea:.6f} rel_err={rel_a:.6e}")
        rb, eb = _bbox(rs2), _bbox(es2)
        bd = max(abs(rb[j] - eb[j]) for j in range(6))
        if bd > 0.1:
            diffs.append(f"Solid[{i}] BBox diff={bd:.6f}")
        for ttype, tname in [(TopAbs_FACE, "Faces"), (TopAbs_EDGE, "Edges"),
                              (TopAbs_VERTEX, "Vertices")]:
            rc2, ec2 = _count(rs2, ttype), _count(es2, ttype)
            if rc2 != ec2:
                diffs.append(f"Solid[{i}] {tname}: ref={rc2} exp={ec2}")
        if not brief:
            rf = _face_info(rs2); ef2 = _face_info(es2)
            for j in range(min(len(rf), len(ef2))):
                r_area, r_type = rf[j]; e_area, e_type = ef2[j]
                area_rel = abs(r_area - e_area) / max(abs(r_area), 1e-15)
                if r_type != e_type:
                    diffs.append(f"  Face[{j}] type: ref={r_type} exp={e_type}")
                elif area_rel > 1e-3:
                    diffs.append(f"  Face[{j}] area rel_err={area_rel:.6e}")

    print(json.dumps({
        "status": "ok",
        "valid": valid,
        "vmsg": vmsg,
        "diffs": diffs,
    }))


# ──────────────────────────────────────────────
# 公开 API：在子进程中执行 OCC，主进程解析结果
# ──────────────────────────────────────────────
def read_step(path):
    """兼容旧接口，直接在当前进程读（仅供单独调用 step_compare.py 时使用）"""
    from OCP.STEPControl import STEPControl_Reader
    r = STEPControl_Reader()
    st = r.ReadFile(path)
    if str(st) != "IFSelect_ReturnStatus.IFSelect_RetDone":
        return None
    r.TransferRoots()
    shape = r.OneShape()
    r.WS().Model().Clear()
    return shape


def compare_files(ref_path, exp_path, verbose=False, brief=False,
                  ref_shape=None, exp_shape=None):
    """在子进程中执行 OCC 对比，主进程不持有任何 OCC 对象。
    ref_shape/exp_shape 参数保留以兼容旧接口（子进程模式下忽略）。
    返回 (passed: bool|None, summary: str)
    """
    cmd = [
        sys.executable, __file__,
        "--worker", ref_path, exp_path,
        "--brief" if brief else "--no-brief",
    ]
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=180)
    except subprocess.TimeoutExpired:
        return False, "compare timeout"
    except Exception as e:
        return False, f"compare subprocess error: {e}"

    stdout = result.stdout.strip()
    if not stdout:
        return False, f"compare worker no output (stderr: {result.stderr[:200]})"

    try:
        data = json.loads(stdout)
    except Exception:
        return False, f"compare worker bad output: {stdout[:200]}"

    status = data.get("status")
    if status == "skip":
        return None, data.get("msg", "skipped")
    if status == "fail":
        return False, data.get("msg", "failed")

    valid = data.get("valid", True)
    vmsg = data.get("vmsg", "OK")
    diffs = data.get("diffs", [])

    if not valid:
        return False, f"validity: {vmsg}"

    if verbose:
        print(f"Reference: {ref_path}")
        print(f"Exported:  {exp_path}")
        if diffs:
            print("=== DIFFERENCES FOUND ===")
            for d in diffs: print(d)
        else:
            print("=== ALL SOLIDS MATCH ===")

    passed = len(diffs) == 0
    summary = "; ".join(diffs) if diffs else "OK"
    return passed, summary


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--worker", action="store_true")
    parser.add_argument("--brief", action="store_true", default=True)
    parser.add_argument("--no-brief", dest="brief", action="store_false")
    parser.add_argument("files", nargs="*")
    args = parser.parse_args()

    if args.worker:
        if len(args.files) < 2:
            print(json.dumps({"status": "fail", "msg": "need 2 file args"}))
            sys.exit(1)
        _worker_read_and_compare(args.files[0], args.files[1], args.brief)
        return

    # 普通命令行调用
    if len(args.files) < 2:
        print(f"Usage: {sys.argv[0]} <original.step> <exported.step>")
        sys.exit(1)
    passed, summary = compare_files(args.files[0], args.files[1], verbose=True, brief=False)
    sys.exit(0 if passed else 1)


if __name__ == "__main__":
    main()
