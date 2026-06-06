# Installation

1. [Executable](#executable)
2. [C++](#c)
3. [Python](#python)

## Executable

mxslc++ does not currently have any pre-built binaries. Follow these steps to build them from source:  
  
Requirements:
* Git
* CMake >= 3.28
* C++17 compiler
* MaterialX installed locally
```bash
git clone https://github.com/jakethorn/ShadingLanguageX.git
cd ShadingLanguageX
cd mxslc++
cmake -S . -B build -DMTLX_ROOT=/path/to/MaterialX/install -DBUILD_TESTING=OFF
cmake --build build --config Release
```

## C++

The above instructions to build the executable also builds the mxslc_lib library.

## Python

Python bindings can be downloaded using pip:
```
pip install mxslcxx
```
OS: Linux and Windows.  
Python version: 3.9, 3.10, 3.11, 3.12, and 3.13.
Example:
```python
import mxslc

mtlx = mxslc.compile_string_to_string('float f = randomfloat() + 1.0;`)
print(mtlx)
```
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <randomfloat name="node1" type="float" />
  <add name="node2" type="float">
    <input name="in1" type="float" nodename="node1" />
    <input name="in2" type="float" value="1" />
  </add>
</materialx>
```
Note that the package name is `mxslcxx`, but the module is called `mxslc` to be consistent with the old compiler.
