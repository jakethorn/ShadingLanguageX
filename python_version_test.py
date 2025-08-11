#!/usr/bin/env python3
"""
Compatibility test script for Python 3.11, 3.12, and 3.13
This script helps verify that mxslc works correctly with supported Python versions.
"""

import sys
import subprocess
from pathlib import Path

def run_command(cmd, description):
    """Run a command and return success status"""
    print(f"{description}")
    print(f"Running: {' '.join(cmd)}")
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
        if result.returncode == 0:
            print("- SUCCESS")
            return True
        else:
            print(f"- FAILED with return code {result.returncode}")
            if result.stderr:
                print(f"Error output: {result.stderr}")
            return False
    except subprocess.TimeoutExpired:
        print("- TIMEOUT")
        return False
    except Exception as e:
        print(f"- ERROR: {e}")
        return False

def test_python_version(python_cmd):
    """Test mxslc with a specific Python version"""
    print(f"{'='*60}")
    
    # Handle compound commands (like "py -3.12")
    cmd_parts = python_cmd.split()
    
    # Check Python version
    version_check = run_command(cmd_parts + ["--version"], "Checking Python version")
    if not version_check:
        return False
        
    # Test import
    import_test = run_command(cmd_parts + ["-c", "import mxslc; print('Import successful')"], 
                             "Testing mxslc import")
    if not import_test:
        return False
    
    # Run a simple test
    simple_test = run_command(cmd_parts + ["-m", "pytest", "tests/test_main.py::test_main_does_not_raise_compile_error", "-v"],
                             "Running simple test")
    
    return simple_test

def main():
    """Main test function"""
    print("MXSLC Python Compatibility Test")
    print("Testing compatibility with Python 3.11, 3.12, and 3.13")
    print(f"Platform: {sys.platform}")
    
    # Change to the mxslc subfolder
    script_dir = Path(__file__).parent / "mxslc"
    original_dir = Path.cwd()
    
    try:
        import os
        os.chdir(script_dir)
        
        success_count = 0
        tested_versions = []
        untested_version = []
        
        # Get the default Python version for comparison
        default_python_version = None
        try:
            result = subprocess.run(["python", "--version"], capture_output=True, timeout=5)
            if result.returncode == 0:
                default_python_version = result.stdout.decode().strip()
        except:
            pass
        
        # Test with default Python
        print(f"{'='*60}")
        print("Testing with default 'python' command...")
        if test_python_version("python"):
            success_count += 1
            tested_versions.append(default_python_version or "python (version unknown)")
        
        # Test specific Python versions
        target_versions = ["3.11", "3.12", "3.13"]
        
        for version in target_versions:
            print(f"{'='*60}")
            print(f"Testing Python {version}...")
            
            # Build command based on platform
            if sys.platform == "win32":
                python_cmd = f"py -{version}"
            else:
                python_cmd = f"python{version}"
            
            try:
                # Check if this Python version is available
                cmd_parts = python_cmd.split() + ["--version"]
                result = subprocess.run(cmd_parts, capture_output=True, timeout=5)
                
                if result.returncode == 0:
                    version_output = result.stdout.decode().strip()
                    print(f"Found: {version_output}")
                    
                    # Skip if this is the same as the default Python we already tested
                    if version_output == default_python_version:
                        print(f"[INFO] Skipping - same as default Python version")
                        continue
                    
                    # Test this version
                    if test_python_version(python_cmd):
                        success_count += 1
                        tested_versions.append(version_output)
                    else:
                        print(f"- Python {version} failed compatibility tests")
                        tested_versions.append(f"{version_output} (FAILED)")
                else:
                    print(f"[INFO] Python {version} not available on this system")
                    untested_version.append(version)
                    
            except Exception as e:
                print(f"[INFO] Python {version} not available: {e}")
                continue
            
        print(f"{'='*60}")
        print(f"Test Summary:")
        print(f"- Tested versions: {len(tested_versions)}")
        for version in tested_versions:
            print(f"  * {version}")
        print(f"- Untested versions: {len(untested_version  )}")
        for version in untested_version:
            print(f"  * {version}")

        print(f"- Successful test runs: {success_count}")
        
        if success_count > 0:
            print("- mxslc appears to be compatible with available Python versions")
            return 0
        else:
            print("- No Python versions passed compatibility tests")
            return 1
            
    except Exception as e:
        print(f"- Test script error: {e}")
        return 1
    finally:
        os.chdir(original_dir)

if __name__ == "__main__":
    sys.exit(main())
