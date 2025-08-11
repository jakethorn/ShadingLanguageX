#!/bin/bash

echo "Installing mxslc ..."

# Check if Python is installed
if ! command -v python3 &> /dev/null; then
    if ! command -v python &> /dev/null; then
        echo "- Error: Python is not installed or not in PATH"
        echo "- Please install Python 3.12 or 3.13 and try again"
        exit 1
    else
        PYTHON_CMD="python"
    fi
else
    PYTHON_CMD="python3"
fi

# Check Python version
$PYTHON_CMD -c "import sys; exit(0 if sys.version_info >= (3, 11) else 1)"
if [ $? -ne 0 ]; then
    echo "- Error: Python 3.11 or higher required"
    echo "- Current version:" $PYTHON_CMD --version
    exit 1
fi

echo "- Have supported Python version: $($PYTHON_CMD --version)"

# Navigate to script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
pushd . > /dev/null
cd "$SCRIPT_DIR/mxslc" > /dev/null

# Install package in editable mode
echo "- Installing mxslc package..."
pip_output=$(pip install -e . 2>&1 > /dev/null)
if [ $? -ne 0 ]; then
    echo "- Error: Failed to install mxslc package"
    echo "$pip_output"
    popd
    exit 1
else
    echo "- mxslc version installed: $(pip show mxslc | grep Version | cut -d ' ' -f 2)"
fi

# Test installation
$PYTHON_CMD -c "import mxslc; print('- mxslc imported successfully')"
if [ $? -ne 0 ]; then
    echo "- Warning: Installation may not be complete"
else
    echo "- Installation verified"
fi

popd
