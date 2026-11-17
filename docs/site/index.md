# ShadingLanguageX

**ShadingLanguageX (SLX)** is a high level programming language for [MaterialX](https://materialx.org/) that makes it
easier to express complex shading algorithms. SLX source code compiles to MaterialX (`.mtlx`) node graphs, and MaterialX
documents decompile back to SLX. It is free, open-source and available for Windows, Linux, macOS and web applications.

<p><a href="compiler/"><strong>Open the compiler</strong></a> to compile SLX to MaterialX, or decompile MaterialX to SLX, in your browser.</p>

## Install

```bash
pip install mxslcxx
```

```python
import mxslc

mtlx = mxslc.compile_string_to_string('surfacematerial(standard_surface(base_color=color3{1, 0, 0}));')
```

The compiler is written in C++, with Python and JavaScript (WebAssembly) bindings.

## Where to start

- [Hello, ShadingLanguageX!](examples/HelloSLX.md) is a first introduction to the language.
- The [User Guide](docs/UserGuide.md) covers installing and running the compiler.
- The [Language Specification](docs/LanguageSpecification.md) describes the language in full.
- The [API guide](docs/APIDocumentation.md) shows how to use the compiler from Python and C++, and the
  [Python](api/python/index), [C++](api/cpp.md) and [JavaScript](api/js.md) references list every function and class.

```{toctree}
:caption: Learn
:hidden:

examples/HelloSLX
docs/UserGuide
docs/LanguageSpecification
docs/BasicExamples
docs/VariableMutability
```

```{toctree}
:caption: API
:hidden:

docs/APIDocumentation
api/python/index
api/cpp
api/js
```
