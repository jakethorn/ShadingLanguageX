#!/bin/bash

set -e

echo "Navigating to project directory..."
cd ~/projects/MXSL/mxslc++

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
python -m build --wheel --config-setting=cmake.define.MTLX_ROOT=/home/jaket/projects/MaterialX-1.39.4/cmake-build-debug/installed

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

echo "Installing MaterialX package..."
python -m pip install --upgrade MaterialX
