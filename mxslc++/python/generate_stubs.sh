#!/usr/bin/env bash
#
# Regenerates python/mxslc/_mxslc.pyi, the type stubs for the compiled
# module, from the mxslc package installed in the active Python environment.
# Run it after installing a freshly built wheel, then commit the result.
# build_python.sh runs it after building and installing the wheel.
# CI (build-wheels.yml) fails if the committed stubs are out of date.
#
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")/.."

python -m pip install --quiet pybind11-stubgen==3.0.0

# Fails if a signature has a C++ type name, e.g. mxslc::preprocess::Macro,
# which pybind11 writes for a type that is bound after the function using it.
# --enum-class-locations lets stubgen write enum default values as Python.
python -m pybind11_stubgen mxslc._mxslc \
    --output-dir python \
    --exit-code \
    --enum-class-locations 'FunctionBehaviour:mxslc._mxslc'

# The module's version would otherwise be baked into the stub, making it stale
# after every version bump.
sed -i 's/^__version__: str = .*/__version__: str/' python/mxslc/_mxslc.pyi

echo "Wrote python/mxslc/_mxslc.pyi"
