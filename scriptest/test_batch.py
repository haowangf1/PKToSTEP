#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Batch test script for PKToSTEP converter
Tests all STEP files in the specified directory
"""

import os
import sys
import subprocess
from pathlib import Path
from datetime import datetime

# Configuration
TEST_RESOURCE_DIR = r"D:\workspace\resource\bugstepfiles"
EXE_PATH = r"..\build\Debug\Debug\PKToSTEP.exe"
RESULT_FILE = "test_results.txt"

def find_step_files(directory):
    """Find all .step and .stp files in the directory"""
    step_files = []
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.lower().endswith(('.step', '.stp')):
                step_files.append(os.path.join(root, file))
    return sorted(step_files)

def run_test(exe_path, step_file):
    """Run PKToSTEP.exe with the given STEP file"""
    try:
        result = subprocess.run(
            [exe_path, step_file],
            capture_output=True,
            text=True,
            timeout=60,  # 60 seconds timeout
            encoding='utf-8',
            errors='replace'
        )
        return result.returncode, result.stdout, result.stderr
    except subprocess.TimeoutExpired:
        return -1, "", "Timeout (60s)"
    except Exception as e:
        return -2, "", str(e)

def main():
    # Check if exe exists
    if not os.path.exists(EXE_PATH):
        print(f"Error: Executable not found: {EXE_PATH}")
        return 1

    # Check if test directory exists
    if not os.path.exists(TEST_RESOURCE_DIR):
        print(f"Error: Test directory not found: {TEST_RESOURCE_DIR}")
        return 1

    # Find all STEP files
    step_files = find_step_files(TEST_RESOURCE_DIR)
    if not step_files:
        print(f"No STEP files found in {TEST_RESOURCE_DIR}")
        return 1

    print(f"Found {len(step_files)} STEP files")
    print(f"Executable: {EXE_PATH}")
    print(f"Test directory: {TEST_RESOURCE_DIR}")
    print(f"Results will be saved to: {RESULT_FILE}")
    print("=" * 80)

    # Run tests
    failed_tests = []
    passed_tests = []

    with open(RESULT_FILE, 'w', encoding='utf-8') as f:
        # Write header
        f.write(f"PKToSTEP Batch Test Results\n")
        f.write(f"Test Time: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n")
        f.write(f"Executable: {EXE_PATH}\n")
        f.write(f"Test Directory: {TEST_RESOURCE_DIR}\n")
        f.write(f"Total Files: {len(step_files)}\n")
        f.write("=" * 80 + "\n\n")

        for idx, step_file in enumerate(step_files, 1):
            rel_path = os.path.relpath(step_file, TEST_RESOURCE_DIR)
            print(f"[{idx}/{len(step_files)}] Testing: {rel_path}...", end=' ')

            returncode, stdout, stderr = run_test(EXE_PATH, step_file)

            if returncode == 0:
                print("PASS")
                passed_tests.append(rel_path)
            else:
                print(f"FAIL (return code: {returncode})")
                failed_tests.append((rel_path, returncode))
                # Write failed test to file
                f.write(f"{rel_path}\t{returncode}\n")

        # Write summary
        f.write("\n" + "=" * 80 + "\n")
        f.write(f"Summary:\n")
        f.write(f"  Total: {len(step_files)}\n")
        f.write(f"  Passed: {len(passed_tests)}\n")
        f.write(f"  Failed: {len(failed_tests)}\n")
        f.write(f"  Success Rate: {len(passed_tests)/len(step_files)*100:.2f}%\n")

    # Print summary
    print("=" * 80)
    print(f"Test Summary:")
    print(f"  Total: {len(step_files)}")
    print(f"  Passed: {len(passed_tests)}")
    print(f"  Failed: {len(failed_tests)}")
    print(f"  Success Rate: {len(passed_tests)/len(step_files)*100:.2f}%")

    if failed_tests:
        print(f"\nFailed tests saved to: {RESULT_FILE}")
        print("\nFailed files:")
        for file, code in failed_tests[:10]:  # Show first 10
            print(f"  {file} (return code: {code})")
        if len(failed_tests) > 10:
            print(f"  ... and {len(failed_tests) - 10} more")

    return 0 if len(failed_tests) == 0 else 1

if __name__ == "__main__":
    sys.exit(main())
