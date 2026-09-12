# mxslc++

This package contains the Python bindings for the ShadingLanguageX C++ compiler. See the
[User Guide](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/UserGuide.md) and
[Basic Examples](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/BasicExamples.md) for information on how to get started and the
[Language Specification](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/LanguageSpecification.md) for documentation about the syntax and features of ShadingLanguageX.

```
pip install mxslcxx
```

OS: Linux, MacOS and Windows.  
Python version: 3.9+.

```python
import mxslc
mtlx = mxslc.compile_string_to_string("float f = randomfloat() + 1.0;")
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

Note that the package name is mxslcxx, but the module is called mxslc to be consistent with the old compiler. See below for the API documentation for the Python bindings.