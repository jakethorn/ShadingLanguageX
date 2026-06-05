# mxslc++

This is the C++ compiler for ShadingLanguageX, it is the successor of the Python compiler. See 
[here](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md) for documentation of the compilers features.

## Python

Python bindings are generated using pybind11 and uploaded to PyPI: https://pypi.org/project/mxslcxx/.

Supported OS: Linux, Windows.  
Supported Python versions: 3.9+.

`pip install mxslcxx`
```
import mxslc

slx = 'float f = randomfloat() + 1.0;'
mtlx = mxslc.compile_string_to_string(slx)
print(mtlx)
```
