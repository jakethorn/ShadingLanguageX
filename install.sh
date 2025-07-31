#!/bin/bash

echo "Installing ShadingLanguageX (mxslc)..."
echo

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    if ! command -v python &> /dev/null; then
        echo "Error: Python is not installed or not in PATH"
        echo "Please install Python 3.12 or 3.13 and try again"
        exit 1
    else
        PYTHON_CMD="python"
    fi
else
    PYTHON_CMD="python3"
fi

# Check Python version
echo "Checking Python version..."
$PYTHON_CMD -c "import sys; exit(0 if sys.version_info >= (3, 12) else 1)"
if [ $? -ne 0 ]; then
    echo "Error: Python 3.12 or higher required"
    echo "Current version:"
    $PYTHON_CMD --version
    exit 1
fi

echo "Python version OK"
echo

# Navigate to script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
pushd .
cd "$SCRIPT_DIR/mxslc"

# Install dependencies
echo "Installing dependencies..."
pip install -r requirements.txt
if [ $? -ne 0 ]; then
    echo "Error: Failed to install dependencies"
    popd
    exit 1
fi

# Install package in editable mode
echo "Installing mxslc package..."
pip install -e .
if [ $? -ne 0 ]; then
    echo "Error: Failed to install mxslc package"
    popd
    exit 1
fi

# Test installation
echo "Testing installation..."
$PYTHON_CMD -c "import mxslc; print('- mxslc imported successfully')"
if [ $? -ne 0 ]; then
    echo "Warning: Installation may not be complete"
else
    echo "- Installation verified"
fi

popd
