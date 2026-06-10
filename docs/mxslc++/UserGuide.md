<h1 align="center">User Guide</h1>

1. [What is ShadingLanguageX?](#what-is-shadinglanguagex)
2. [Useful Features](#useful-features)
3. [Installation](#installation)
4. [Getting Started](#getting-started)
5. [API Documentation](#api-documentation)

# What is ShadingLanguageX?

ShadingLanguageX is a high level programming language that can be used to create complex MaterialX shaders. Statements
and expressions are compiled down to native MaterialX elements, like Nodes and NodeDefs. For example, 
`int i = 2 + 2;` would compile to:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <add name="node1" type="int">
    <input name="in1" type="int" value="2" />
    <input name="in2" type="int" value="2" />
  </add>
</materialx>
```

ShadingLanguageX is most useful for complex shaders, providing features such as loops, functions, classes. It also
supports preprocessor directives like `#include` to re-use code across shaders and `#define` `#if` `#ifdef` to create 
shader variants.
You can see more in-depth examples [here]() and the language specification [here]().  
ShadingLanguageX files are compiled to .mtlx files using the `mxslc` compiler. See below for details on how to install
and get started using `mxslc`.

# Useful Features

TODO

# Installation

TODO

# Getting Started

TODO

# API Documentation

## C++

### Structs

```
namespace mxslc
{
    struct CompileOptions
    {
        // Save location of the compiled MaterialX file.
        optional<fs::path> output_file = std::nullopt;
        
        // MaterialX version to compile against.
        string version = "1.39.5";
        
        // If true, pre-computes some operations during compilation.
        bool reduce_graph = true;
    };
}

// Example
mxslc::CompileOptions opts;
opts.output_file = fs::path{"example.mtlx"};
opts.version = "1.39.4";
opts.reduce_graph = false;
```

### Functions

```
namespace mxslc 
{
    MaterialX::DocumentPtr compile_to_document(const string& source);
    MaterialX::DocumentPtr compile_to_document(const string& source, const CompileOptions& opts);
    MaterialX::DocumentPtr compile_to_document(const fs::path& src_path);
    MaterialX::DocumentPtr compile_to_document(const fs::path& src_path, const CompileOptions& opts);
    
    string compile_to_string(const string& source);
    string compile_to_string(const string& source, const CompileOptions& opts);
    string compile_to_string(const fs::path& src_path);
    string compile_to_string(const fs::path& src_path, const CompileOptions& opts);
    
    fs::path compile_to_file(const string& source, const CompileOptions& opts);
    fs::path compile_to_file(const string& source, const fs::path& dst_path);
    fs::path compile_to_file(const fs::path& src_path);
    fs::path compile_to_file(const fs::path& src_path, const CompileOptions& opts);
    fs::path compile_to_file(const fs::path& src_path, const fs::path& dst_path);
}
```

```
// Example
mxslc::CompileOptions opts;
opts.reduce_graph = false;
string mtlx = mxslc::compile_to_string("float f = 1.0 + 1.0;"s, opts);

fs::path src_path{"example.mtlx"};
fs::path dst_path = mxslc::compile_file_to_file(src_path);
```
## Python

### Classes

```
mxslc.CompileOptions

### Fields ###

# Save location of the compiled MaterialX file.
# Default value = None
mxslc.CompileOptions.output_file: pathlib.Path

# MaterialX version to compile against.
# Default value = "1.39.5"
mxslc.CompileOptions.version: str

# If true, pre-computes some operations during compilation.
# Default value = True
mxslc.CompileOptions.reduce_graph: bool
```

```
# Example
opts = mxslc.CompileOptions()
opts.output_file = pathlib.Path("example.mtlx")
opts.version = "1.39.4"
opts.reduce_graph = False
```

### Functions

```
mxslc.compile_string_to_string(source: str) -> str
mxslc.compile_string_to_string(source: str, opts: mxslc.CompileOptions) -> str

mxslc.compile_file_to_string(src_path: pathlib.Path) -> str
mxslc.compile_file_to_string(src_path: pathlib.Path, opts: mxslc.CompileOptions) -> str

mxslc.compile_string_to_file(source: str, opts: mxslc.CompileOptions) -> pathlib.Path
mxslc.compile_string_to_file(source: str, dst_path: pathlib.Path) -> pathlib.Path

mxslc.compile_file_to_file(src_path: pathlib.Path) -> pathlib.Path
mxslc.compile_file_to_file(src_path: pathlib.Path, opts: mxslc.CompileOptions) -> pathlib.Path
mxslc.compile_file_to_file(src_path: pathlib.Path, dst_path: pathlib.Path) -> pathlib.Path
```

```
# Example
opts = mxslc.CompileOptions()
opts.reduce_graph = False
mtlx = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", opts)

src_path = pathlib.Path("example.mtlx")
dst_path = mxslc.compile_file_to_file(src_path)
```