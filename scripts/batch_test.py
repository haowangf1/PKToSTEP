"""
STEP 批量导入导出验证脚本

用法:
  python scripts/batch_test.py [step目录] [--exe <PKToSTEP路径>] [--output-dir <结果目录>]

  不给 step目录 时默认用项目的 resource/ 目录

结果:
  passed.txt  - 通过的文件名（每行一个）
  failed.txt  - 失败的文件名 + 原因（tab 分隔）
  skipped.txt - OCC 读不了原始文件的
"""
import sys
import os
import subprocess
import glob
import argparse

# 让 import step_compare 能找到同目录
sys.path.insert(0, os.path.dirname(__file__))
from step_compare import compare_files, read_step

from OCP.BRepCheck import BRepCheck_Analyzer
from OCP.TopExp import TopExp_Explorer
from OCP.TopAbs import TopAbs_SOLID, TopAbs_FACE
from OCP.TopoDS import TopoDS


def check_validity(step_path):
    """检查导出文件中所有 solid/face 的有效性，返回 (valid, summary)"""
    shape = read_step(step_path)
    if shape is None:
        return False, "OCC cannot read export"

    # 检查是否有 solid
    exp_s = TopExp_Explorer(shape, TopAbs_SOLID)
    solid_count = 0
    while exp_s.More():
        solid_count += 1
        exp_s.Next()
    if solid_count == 0:
        return False, "no solids in export"

    # 检查每个 face 的有效性
    invalid_faces = 0
    total_faces = 0
    exp_f = TopExp_Explorer(shape, TopAbs_FACE)
    while exp_f.More():
        face = TopoDS.Face_s(exp_f.Current())
        total_faces += 1
        analyzer = BRepCheck_Analyzer(face)
        if not analyzer.IsValid():
            invalid_faces += 1
        exp_f.Next()

    if invalid_faces > 0:
        return False, f"{invalid_faces}/{total_faces} invalid faces"
    return True, "OK"

# 项目根目录
PROJECT_ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))
DEFAULT_EXE = os.path.join(PROJECT_ROOT, "build", "Debug", "Debug", "PKToSTEP.exe")
DEFAULT_STEP_DIR = os.path.join(PROJECT_ROOT, "resource")
DEFAULT_OUTPUT_DIR = os.path.join(PROJECT_ROOT, "stepexport")


def find_exe():
    if os.path.exists(DEFAULT_EXE):
        return os.path.abspath(DEFAULT_EXE)
    return None


def collect_files(step_dir):
    files = sorted(
        glob.glob(os.path.join(step_dir, "*.step")) +
        glob.glob(os.path.join(step_dir, "*.stp")) +
        glob.glob(os.path.join(step_dir, "*.STEP")) +
        glob.glob(os.path.join(step_dir, "*.STP"))
    )
    seen = set()
    unique = []
    for f in files:
        key = os.path.normcase(f)
        if key not in seen:
            seen.add(key)
            unique.append(f)
    return unique


def main():
    parser = argparse.ArgumentParser(description="STEP batch export & compare")
    parser.add_argument("step_dir", nargs="?", default=DEFAULT_STEP_DIR,
                        help="Directory containing .step/.stp files (default: resource/)")
    parser.add_argument("--exe", default=None, help="Path to PKToSTEP.exe")
    parser.add_argument("--output-dir", default=DEFAULT_OUTPUT_DIR, help="Output directory for result files")
    args = parser.parse_args()

    exe = args.exe or find_exe()
    if not exe:
        print(f"[ERROR] PKToSTEP.exe not found at {DEFAULT_EXE}. Use --exe to specify.")
        sys.exit(1)

    # 确保输出目录存在
    os.makedirs(args.output_dir, exist_ok=True)

    files = collect_files(args.step_dir)
    print(f"Exe: {exe}")
    print(f"Dir: {args.step_dir}")
    print(f"Files: {len(files)}")
    print("=" * 70)

    passed_list = []
    failed_list = []
    skipped_list = []

    for i, fpath in enumerate(files):
        fname = os.path.basename(fpath)
        stem = os.path.splitext(fname)[0]
        export_name = os.path.join(args.output_dir, f"{stem}_export.step")
        tag = f"[{i+1}/{len(files)}]"

        # 1. 导出（cwd 设为 output-dir，使 exe 输出到该目录）
        try:
            result = subprocess.run(
                [exe, fpath], capture_output=True, text=True, timeout=120,
                cwd=args.output_dir)
        except subprocess.TimeoutExpired:
            print(f"{tag} FAIL {fname}: export timeout")
            failed_list.append((fname, "export timeout"))
            continue
        except Exception as e:
            print(f"{tag} FAIL {fname}: export error: {e}")
            failed_list.append((fname, f"export error: {e}"))
            continue

        if result.returncode != 0:
            print(f"{tag} FAIL {fname}: export failed (rc={result.returncode})")
            failed_list.append((fname, f"export failed (rc={result.returncode})"))
            continue

        if not os.path.exists(export_name):
            print(f"{tag} FAIL {fname}: export file not found")
            failed_list.append((fname, "export file not found"))
            continue

        # 2. OCC 对比（直接调用 step_compare）
        try:
            passed, summary = compare_files(fpath, export_name, brief=True)
        except Exception as e:
            passed, summary = False, f"compare error: {e}"

        # 清理导出文件（注释掉以便手动查看）
        # if os.path.exists(export_name):
        #     os.remove(export_name)

        if passed is None:
            # OCC 读不了原始文件
            print(f"{tag} SKIP {fname}: {summary}")
            skipped_list.append((fname, summary))
        elif passed:
            # 3. 对比通过后，检查导出文件的 face 有效性
            valid, vsummary = check_validity(export_name)
            if valid:
                print(f"{tag} PASS {fname}")
                passed_list.append(fname)
            else:
                print(f"{tag} FAIL {fname}: {vsummary}")
                failed_list.append((fname, vsummary))
        else:
            print(f"{tag} FAIL {fname}: {summary}")
            failed_list.append((fname, summary))

    # 写结果
    print()
    print("=" * 70)
    print(f"PASSED: {len(passed_list)}  FAILED: {len(failed_list)}  SKIPPED: {len(skipped_list)}")

    out = args.output_dir
    with open(os.path.join(out, "passed.txt"), "w", encoding="utf-8") as f:
        for name in passed_list:
            f.write(name + "\n")
    with open(os.path.join(out, "failed.txt"), "w", encoding="utf-8") as f:
        for name, reason in failed_list:
            f.write(name + "\n")
    if skipped_list:
        with open(os.path.join(out, "skipped.txt"), "w", encoding="utf-8") as f:
            for name, reason in skipped_list:
                f.write(f"{name}\t{reason}\n")

    print(f"Results: {out}/passed.txt, {out}/failed.txt")


if __name__ == "__main__":
    main()
