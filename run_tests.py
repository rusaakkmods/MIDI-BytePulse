#!/usr/bin/env python3
"""
Pre-upload script to run unit tests before uploading firmware.
This ensures code quality by verifying all tests pass before flashing the device.
"""
import subprocess
import sys
import os

Import("env")

def run_unit_tests(*args, **kwargs):
    """Run all unit tests in the native environment."""
    print("\n" + "="*80)
    print("Running unit tests before upload...")
    print("="*80 + "\n")
    
    # Ensure MinGW is in PATH for Windows
    if sys.platform == "win32":
        mingw_path = "C:\\msys64\\mingw64\\bin"
        if os.path.exists(mingw_path):
            os.environ["PATH"] = mingw_path + os.pathsep + os.environ.get("PATH", "")
    
    # Run tests
    try:
        result = subprocess.run(
            ["pio", "test", "-e", "native"],
            capture_output=False,
            text=True,
            check=True
        )
        
        print("\n" + "="*80)
        print("✅ All tests passed! Proceeding with upload...")
        print("="*80 + "\n")
        return 0
        
    except subprocess.CalledProcessError as e:
        print("\n" + "="*80)
        print("❌ Tests failed! Upload cancelled.")
        print("Fix the failing tests before uploading to the device.")
        print("="*80 + "\n")
        env.Exit(1)
    except FileNotFoundError:
        print("\n" + "="*80)
        print("⚠️  Warning: Could not run tests (pio command not found)")
        print("Proceeding with upload anyway...")
        print("="*80 + "\n")
        return 0

# Register the pre-upload action
env.AddPreAction("upload", run_unit_tests)
