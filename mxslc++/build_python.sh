#!/bin/bash
#
# Builds the mxslcxx Python wheel on macOS / Linux, regenerates the Python
# stubs (python/mxslc/_mxslc.pyi) and runs the Python tests.
#
# Usage:
#   build_python.sh <materialx_install>
#
# The MaterialX install (the directory containing include/ and lib/) is
# required: pass it as the first argument or set the MTLX_ROOT environment
# variable.
#

set -e

MTLX_ROOT="${1:-${MTLX_ROOT:-}}"
if [ -z "$MTLX_ROOT" ]; then
    echo "Error: MaterialX install folder is required. Provide it as the first argument or set MTLX_ROOT." >&2
    exit 1
fi
if [ ! -d "$MTLX_ROOT" ]; then
    echo "Error: MaterialX install folder not found at '$MTLX_ROOT'." >&2
    exit 1
fi
MTLX_ROOT="$(cd "$MTLX_ROOT" && pwd)"

echo "Navigating to project directory..."
cd "$(dirname "${BASH_SOURCE[0]}")"

echo "Cleaning old build artifacts..."
rm -rf .build-venv
rm -rf .test-venv
rm -rf build
rm -rf dist

echo "Creating build virtual environment..."
python3 -m venv .build-venv
source .build-venv/bin/activate

echo "Installing build dependencies..."
python -m pip install --upgrade build pybind11 scikit-build-core twine

echo "Building the wheel..."
python -m build --wheel --config-setting=cmake.define.MTLX_ROOT="$MTLX_ROOT"

# 1. Grab the full path to the wheel file
WHEEL_PATH=$(ls dist/*.whl)
# 2. Extract just the filename itself (removes the "dist/" part)
WHEEL_NAME=$(basename "$WHEEL_PATH")
echo "Success! The newly created wheel is named: $WHEEL_NAME"

echo "Checking the wheel with twine..."
python -m twine check dist/*

echo "Deactivating build virtual environment..."
deactivate

echo "Build complete! Check the /dist folder."

echo "Creating test virtual environment..."
python3 -m venv .test-venv
source .test-venv/bin/activate

echo "Installing test dependencies..."
python -m pip install --upgrade pytest

echo "Installing mxslc..."
python -m pip install "$WHEEL_PATH"

echo "Regenerating the Python stubs..."
bash python/generate_stubs.sh

echo "Running tests without MaterialX module..."
python -m pytest python/tests -vv

echo "Installing MaterialX package..."
python -m pip install --upgrade MaterialX

echo "Running tests with MaterialX module..."
python -m pytest python/tests -vv

echo "Tests complete!"