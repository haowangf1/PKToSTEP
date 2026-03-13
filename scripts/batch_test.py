"""
STEP 批量导入导出验证脚本

用法:
  python scripts/batch_test.py <step目录> [--exe <PKToSTEP路径>] [--output-dir <结果目录>]

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
from step_compare import compare_files


def find_exe():
    candidates = [
        os.path.join(os.path.dirname(__file__), "..", "build", "Debug", "Debug", "PKToSTEP.exe"),
        os.path.join("build", "Debug", "Debug", "PKToSTEP.exe"),
    ]
    for c in candidates:
        if os.path.exists(c):
            return os.path.abspath(c)
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
    parser.add_argument("step_dir", help="Directory containing .step/.stp files")
    parser.add_argument("--exe", default=None, help="Path to PKToSTEP.exe")
    parser.add_argument("--output-dir", default=".", help="Output directory for result files")
    args = parser.parse_args()

    exe = args.exe or find_exe()
    if not exe:
        print("[ERROR] PKToSTEP.exe not found. Use --exe to specify.")
        sys.exit(1)

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
        export_name = f"{stem}_export.step"
        tag = f"[{i+1}/{len(files)}]"

        # 1. 导出
        try:
            result = subprocess.run(
                [exe, fpath], capture_output=True, text=True, timeout=120)
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

        # 清理导出文件
        if os.path.exists(export_name):
            os.remove(export_name)

        if passed is None:
            # OCC 读不了原始文件
            print(f"{tag} SKIP {fname}: {summary}")
            skipped_list.append((fname, summary))
        elif passed:
            print(f"{tag} PASS {fname}")
            passed_list.append(fname)
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
