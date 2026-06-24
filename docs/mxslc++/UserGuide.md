<h1 align="center">User Guide</h1>

<p align="center">
  <img src="https://github.com/jakethorn/ShadingLanguageX/blob/main/examples/screenshots/combined.png" />
</p>

# Table of Contents

1. [What is ShadingLanguageX?](#what-is-shadinglanguagex)
2. [Language Features](#language-features)
3. [Installation](#installation)
4. [Getting Started](#getting-started)
5. [API Documentation](#api-documentation)
6. [Decompiler](#decompiler)
7. [MaterialX Libraries](#materialx-libraries)

# What is ShadingLanguageX?

ShadingLanguageX is a high-level programming language that can be used to create complex MaterialX shaders. Statements
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
You can see more in-depth examples [here](https://github.com/jakethorn/ShadingLanguageX/tree/main/examples) and the language specification [here](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc%2B%2B/LanguageSpecification.md).  
ShadingLanguageX files are compiled to .mtlx files using the `mxslc` compiler. See below for details on how to install
and get started using `mxslc`.

# Language Features

## MaterialX Version

You can easily target a specific MaterialX version either by defining the version in the compiled file or by
passing it as an argument to the compiler. If both are used, the value passed to the compiler takes precedence. The default version is 1.39.5.

```
#version 1.38.10
vec3 n = normal();
float = swizzle(n, "x");
```

## Attributes

ShadingLanguageX has built-in support for adding attributes to MaterialX elements using the `@` symbol.

```
@file.colorspace "srgb_texture"
color3 c = image("butterfly1.png");
```
```xml
<image name="c" type="color3" doc="an image of a butterfly">
    <input name="file" type="filename" colorspace="srgb_texture" value="butterfly1.png" />
</image>
```

Attributes can be added to Nodes (as above), NodeDefs and at the document level (using `@@`).

```
@@madewith "ShadingLanguageX"
standard_surface(base_color = randomcolor());
```
```xml
<?xml version="1.0"?>
<materialx version="1.39" madewith="ShadingLanguageX">
  <randomcolor name="node1" type="color3" />
  <standard_surface name="node2" type="surfaceshader">
    <input name="base_color" type="color3" nodename="node1" />
  </standard_surface>
</materialx>
```

## Loops

ShadingLanguageX typically simply exposes MaterialX functionality in a convenient syntax. However, one thing that
ShadingLanguageX allows that MaterialX does not is loops, with the caveat that the number of iterations must be known at 
compile-time.

```
// brownian noise
float f = 0.0;
for (int i = 0:2)
{
    f += my_noise_func(seed=i);
}
f /= 3.0;
```

## User-Defined Types

ShadingLanguageX supports defining your own custom types. This can help organise data and functionality when creating complex shaders.

```
class Sphere
{
    vec3 position;
    float radius;
    
    Sphere(vec3 position = vec3{0}, float radius = 1)
    {
        this.position = position;
        this.radius = radius;
    }
    
    bool is_inside(vec3 point)
    {
        return distance(point, position) < radius;
    }
}
```

## Compile-Time Operations

Another benefit of using `mxslc` is that it will compute certain operations at compile-time when possible. For example, while 
`int i = 2 + 2;` would typically compile into an `add` node, the compiler can be instructed to compute the value at 
compile-time, reducing the number of nodes in the shader. This isn't always possible however, for example, `int i = geompropvalue<int>("i") + 2;`
is not possible to compute at compile-time because the value of `geompropvalue` is not known until render-time.

# Installation

## Executable

See [Build from Source](#build-from-source).

## C++ Library

See [Build from Source](#build-from-source).

To use `mxslc` in your C++ project, link to the `mxslc_lib` static library and include the `include` folder in your project.

```c++
#include <compile.h>
fs::path src_path{"example.mxsl"};
fs::path dst_path = mxslc::compile_to_file("./example.mxsl");
```

## Python 

Python bindings are available on PyPI and can be downloaded using pip:
```
pip install mxslcxx
```

OS: Linux and Windows.  
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

Note that the package name is mxslcxx, but the module is called mxslc to be consistent with the old compiler. See below
for the API documentation for the Python bindings.

## Build from Source

To build from source, you need:

- a C++17 compiler
- CMake 3.28 or newer
- an installed MaterialX distribution
- Python 3.9+, if building the Python bindings

### Building the C++ Executable and Static Library

Clone the project and navigate to the `mxslc++` directory:

```bash
git clone https://github.com/jakethorn/ShadingLanguageX.git
cd ShadingLanguageX
cd mxslc++
```

Configure the project with CMake, passing the path to your MaterialX installation through `MTLX_ROOT`:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DMTLX_ROOT=/path/to/MaterialX/install -DBUILD_TESTING=OFF
cmake --build build --config Release
```

After building, the `mxslc` executable and `mxslc_lib` static library will be available in the build output directory.

### Building the Python Bindings

Clone the project and navigate to the `mxslc++` directory:

```bash
git clone https://github.com/jakethorn/ShadingLanguageX.git
cd ShadingLanguageX
cd mxslc++
```

Create a Python virtual environment, install the required dependencies and build the wheel:

```bash
sudo apt install python3-pip python3-venv
python3 -m venv .build-venv
source .build-venv/bin/activate
python -m pip install --upgrade pip build scikit-build-core pybind11
python -m build --wheel --config-setting=cmake.define.MTLX_ROOT=/path/to/MaterialX/install
deactivate
```

Test the wheel by installing it in a new virtual environment:

```bash
python3 -m venv .test-venv
source .test-venv/bin/activate
python -m pip install dist/mxslcxx-*.whl
python -c "import mxslc; print(mxslc.compile_string_to_string('float f = randomfloat() + 1.0;'));"
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

# Getting Started

ShadingLanguageX files are compiled using `mxslc`. The compiler is available as an executable, C++ library or Python 
module. The following examples demonstrate `mxslc` by calling the executable from the command line, but the 
same functionality and options are available in the C++ and Python APIs (see below).

The following is a typical ShadingLanguageX file, which uses functions generated from the MaterialX standard library,
such as `texcoord`, `floor` and `randcomcolor`. Finally it creates a material using the `surfacematerial` function. 

```
// example.mxsl

float u, v = texcoord() * 10.0;
float seed = floor(u) + floor(v) * 10.0;
color3 c = randomcolor(seed);

surfacematerial(
    standard_surface(base_color=c)
);
```

The following command will compile `example.mxsl` and create a file called `example.mtlx` in the same location as the source file.

```bash
> ./mxslc example.mxsl
```

```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <texcoord name="node1" type="vector2" />
  <multiply name="node2" type="vector2">
    <input name="in1" type="vector2" nodename="node1" />
    <input name="in2" type="float" value="10" />
  </multiply>
  <separate2 name="node3" type="multioutput">
    <input name="in" type="vector2" nodename="node2" />
  </separate2>
  <floor name="node4" type="float">
    <input name="in" type="float" output="outx" nodename="node3" />
  </floor>
  <floor name="node5" type="float">
    <input name="in" type="float" output="outy" nodename="node3" />
  </floor>
  <multiply name="node6" type="float">
    <input name="in1" type="float" nodename="node5" />
    <input name="in2" type="float" value="10" />
  </multiply>
  <add name="node7" type="float">
    <input name="in1" type="float" nodename="node4" />
    <input name="in2" type="float" nodename="node6" />
  </add>
  <randomcolor name="node8" type="color3">
    <input name="in" type="float" nodename="node7" />
  </randomcolor>
  <standard_surface name="node9" type="surfaceshader">
    <input name="base_color" type="color3" nodename="node8" />
  </standard_surface>
  <surfacematerial name="node10" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="node9" />
  </surfacematerial>
</materialx>
```

![](../../examples/screenshots/squares.png)

## Compile Options

### Output File

To specifiy the output file name, use the `-o/--output-file` option:

```bash
> ./mxslc example.mxsl -o output_file.mtlx
```

### MaterialX Version

To specify the MaterialX library version, use the `-v/--version` option:

```bash
> ./mxslc example.mxsl -v 1.38.10
```

The compiler can only use MaterialX library versions that it can find locally. See [below](#materialx-libraries) for more 
information.

### Entry Function

An optional entry function can be specified using the `-f/--func` option.

```
inline void red()
{
    surfacematerial(
        standard_surface(base_color=color3{1,0,0})
    );
}

inline void green()
{
    surfacematerial(
        standard_surface(base_color=color3{0,1,0})
    );
}

inline void blue()
{
    surfacematerial(
        standard_surface(base_color=color3{0,0,1})
    );
}
```

```bash
> ./mxslc example.mxsl -f blue
```

```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <standard_surface name="node1" type="surfaceshader">
    <input name="base_color" type="color3" value="0, 0, 1" />
  </standard_surface>
  <surfacematerial name="node2" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="node1" />
  </surfacematerial>
</materialx>
```

Arguments can also be passed to the entry function using the `-a/--args` option.

```
inline void main(float r, float g, float b)
{
    surfacematerial(
        standard_surface(base_color=color3{r,g,b})
    );
}
```

```bash
> ./mxslc example.mxsl -f main -a 1 0 0
```

```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <standard_surface name="node1" type="surfaceshader">
    <input name="base_color" type="color3" value="1, 0, 0" />
  </standard_surface>
  <surfacematerial name="node2" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="node1" />
  </surfacematerial>
</materialx>
```

The command line options are limited to passing booleans, integers, floats and strings; however, the C++ and Python APIs
allow vectors, colors and matrices to be passed as well.

### Globals

Certain variables in the shader can also be assigned a value from the command line using the `-g--/globals` option, similar to passing arguments to the 
entry function. These are called global variables.

```
global float metalness;
global float roughness;

standard_surface(
    base_color=randomcolor(), 
    metalness=metalness, 
    specular_roughness=roughness
);
```

```bash
> ./mxslc example.mxsl -g metalness 1.0 roughness 0.2
```

```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <randomcolor name="node1" type="color3" />
  <standard_surface name="node2" type="surfaceshader">
    <input name="base_color" type="color3" nodename="node1" />
    <input name="metalness" type="float" value="1" />
    <input name="specular_roughness" type="float" value="0.2" />
  </standard_surface>
</materialx>
```

Once again, the command line options are limited to passing basic types; however, the C++ and Python APIs
allow vectors, colors, matrices as well as user-defined types to be passed as well (see below).

There are two additional options that are related to globals, `--missing-globals-ok` and `--unused-globals-ok`, which 
suppress compile errors either if a global variable is not provided a value or if a global value is provided, but never used.

### No Reduce Graph

The `--no-reduce-graph` option stops the compiler from trying to optimise operations at compile-time and instead 
computes everything using MaterialX nodes, including variable constructors.

Take the following code for example:

```
standard_surface(base_color = color3{1, 0, 0.5} * 0.5);
```

Compiled without `--no-reduce-graph`:

```xml
<?xml version="1.0"?>
<materialx version="1.39">
    <standard_surface name="node1" type="surfaceshader">
        <input name="base_color" type="color3" value="0.5, 0, 0.25" />
    </standard_surface>
</materialx>
```

Compiled with `--no-reduce-graph`:

```xml
<?xml version="1.0"?>
<materialx version="1.39">
    <combine3 name="node1" type="color3">
        <input name="in1" type="float" value="1" />
        <input name="in2" type="float" value="0" />
        <input name="in3" type="float" value="0.5" />
    </combine3>
    <multiply name="node2" type="color3">
        <input name="in1" type="color3" nodename="node1" />
        <input name="in2" type="float" value="0.5" />
    </multiply>
    <standard_surface name="node3" type="surfaceshader">
        <input name="base_color" type="color3" nodename="node2" />
    </standard_surface>
</materialx>
```

## Decompile

`mxslc` can also decompile MaterialX files into ShadingLanguageX source files (see [below](#decompiler) for more information).

```bash
> ./mxslc decompile example.mtlx -o example_decomped.mxsl
```
_example.mtlx_:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <combine2 name="node1" type="vector2">
    <input name="in1" type="float" value="0" />
    <input name="in2" type="float" value="1" />
  </combine2>
  <add name="node2" type="vector2">
    <input name="in1" type="vector2" nodename="node1" />
    <input name="in2" type="float" value="2" />
  </add>
</materialx>
```
_example_decomped.mxsl_:
```
vector2 node1 = combine2(in1 = float{0}, in2 = float{1}, );
vector2 node2 = node1 + float{2};
```

## Help

Use the `-h/--help` option to see all available options:

```bash
> ./mxslc -h
```
```
actions:
  compile                        Compile the .mxsl file to .mtlx (default)
  decompile                      Decompile the .mtlx file to .mxsl

positional arguments:
  input-file                     Input path to .mxsl file

options:
  -h, --help                     Show this help message and exit
  -o, --output-file OUTPUT_FILE  Output path of .mtlx file
  -v, --version VERSION          Target MaterialX version (default: 1.39.5)
  -f, --func FUNC                Name of entry function into the program
  -a, --args ARGS*               Arguments to be passed to the entry function
  -g, --globals NAME VALUE*      Values to be assigned to `global` variables
  --missing-globals-ok           Allow `global` variables to be missing
  --unused-globals-ok            Allow `global` variables to be unused
  --no-reduce-graph              Always create graph nodes instead of evaluating logic at compile-time
```

## Response File

It's also possible to use a response file instead of manually passing options.

```
# example.rsp
example.mxsl -o output_file.mtlx -v 1.38.10 --no-reduce-graph
--func main
--args 1 0 0 "world"
```

```bash
> ./mxslc @example.rsp
```

# API Documentation

## C++

### Types

#### mxslc::primitive_t

```c++
namespace mxslc
{
    using primitive_t = std::variant<
        bool,
        int,
        float,
        string,
        fs::path,
        MaterialX::Vector2,
        MaterialX::Vector3,
        MaterialX::Vector4,
        MaterialX::Color3,
        MaterialX::Color4,
        MaterialX::Matrix33,
        MaterialX::Matrix44
    >;
}
```

### Classes and Structs

#### mxslc::Variable

```c++
namespace mxslc
{
    class Variable;
    using VariablePtr = std::shared_ptr<Variable>;

    class Variable
    {
    public:
        Variable(primitive_t value);
        Variable(const std::vector<primitive_t>& children);
        Variable(std::vector<VariablePtr> children);
        Variable(std::string name, primitive_t value);
        Variable(std::string name, const std::vector<primitive_t>& children);
        Variable(std::string name, std::vector<VariablePtr> children);
        Variable(std::string type, std::string name, primitive_t value);
        Variable(std::string type, std::string name, const std::vector<primitive_t>& children);
        Variable(std::string type, std::string name, std::vector<VariablePtr> children);
    };
}
```

#### mxslc::CompileOptions

```c++
namespace mxslc
{
    struct CompileOptions
    {
        // Save location of the compiled MaterialX file.
        optional<fs::path> output_file = std::nullopt;
        
        // MaterialX version to compile against.
        string version = "1.39.5";
        
        // Name of entry function into the program
        optional<string> func_name = std::nullopt;
        
        // Arguments to be passed to the entry function
        vector<primitive_t> func_args{};
        
        // Values to be assigned to `global` variables
        std::vector<Variable> globals{};
        
        // If true, throw an error if a `global` variables value is not provided
        bool error_on_missing_globals{true};
        
        // If true, throw an error if a value is provided for a `global` variable that does not exist
        bool error_on_unused_globals{true};
        
        // If true, pre-computes some operations during compilation.
        bool reduce_graph{true};
    };
}
```
```c++
// Example
mxslc::CompileOptions opts;
opts.output_file = fs::path{"example.mtlx"};
opts.version = "1.39.4";
opts.func_name = "main";
opts.func_args = {1.0f, MaterialX::Vector3{1, 0, 0}};
opts.globals = {mxslc::Variable{"metalness", 1.0f}, mxslc::Variable{"roughness", 0.2f}};
opts.reduce_graph = false;
```

#### mxslc::Decompiler

```c++

namespace mxslc
{
    class Decompiler
    {
    public:
        Decompiler(const std::filesystem::path& src_path);
        Decompiler(const std::string& source);
        Decompiler(MaterialX::DocumentPtr document);

        std::string decompile_document();
        std::string decompile_node(const std::string& node_name, bool with_dependencies = false);
        std::string decompile_node(const MaterialX::NodePtr& node, bool with_dependencies = false);
        std::string decompile_node_def(const std::string& node_def_name, bool with_dependencies = false);
        std::string decompile_node_def(const MaterialX::NodeDefPtr& node_def, bool with_dependencies = false);
        std::string decompile_node_graph(const std::string& node_graph_name, bool with_dependencies = false);
        std::string decompile_node_graph(const MaterialX::NodeGraphPtr& node_graph, bool with_dependencies = false);
    };
}
```

The `with_dependencies` argument tells the decompiler to include all Nodes, NodeDefs and NodeGraphs that precede or are used by 
the given element or an element preceding it in the graph.

### Functions

#### mxslc::compile_to_*

```c++
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
```c++
// Example
mxslc::CompileOptions opts;
opts.reduce_graph = false;
string mtlx = mxslc::compile_to_string("float f = 1.0 + 1.0;"s, opts);

fs::path src_path{"example.mxsl"};
fs::path dst_path = mxslc::compile_to_file(src_path);
```

#### mxslc::decompile_to_*

```c++
namespace mxslc
{
    std::string decompile_to_string(const std::filesystem::path& src_path);
    std::string decompile_to_string(const std::string& source);
    std::string decompile_to_string(const MaterialX::DocumentPtr& document);

    std::filesystem::path decompile_to_file(const std::filesystem::path& src_path, const std::optional<std::filesystem::path>& dst_path = std::nullopt);
    std::filesystem::path decompile_to_file(const std::filesystem::path& src_path, const std::filesystem::path& dst_path);
    std::filesystem::path decompile_to_file(const std::string& source, const std::filesystem::path& dst_path);
    std::filesystem::path decompile_to_file(const MaterialX::DocumentPtr& document, const std::filesystem::path& dst_path);
}
```
## Python

### Types

```python
type Primitive = bool | int | float | str | tuple[float] | MaterialX.Vector2 | MaterialX.Vector3 | MaterialX.Vector4 | MaterialX.Color3 | MaterialX.Color4 | MaterialX.Matrix33 | MaterialX.Matrix44]
```

### Classes

### `mxslc.Variable`

#### Constructors

```python
Variable.__init__(                      value: Primitive | list[Primitive] | list[Variable])
Variable.__init__(           name: str, value: Primitive | list[Primitive] | list[Variable])
Variable.__init__(type: str, name: str, value: Primitive | list[Primitive] | list[Variable])
```

### `mxslc.CompileOptions`

#### Constructors

```python
CompileOptions.__init__(
    output_file: str | pathlib.Path | None = None,
    version: str = "1.39.5",
    func_name: str | None = None,
    func_args: list[Primitive] = [],
    globals: list[Variable] = [],
    error_on_missing_globals: bool = True,
    error_on_unused_globals: bool = True,
    reduce_graph: bool = True
)
```

#### Properties

```
# Save location of the compiled MaterialX file. Defaults to None.
CompileOptions.output_file: pathlib.Path

# MaterialX version to compile against. Defaults to "1.39.5".
CompileOptions.version: str

# Name of entry function into the program. Defaults to None.
CompileOptions.func_name: str

# Arguments to be passed to the entry function. Defaults to [].
CompileOptions.func_args: list[Primitive]

# Values to be assigned to `global` variables. Defaults to [].
CompileOptions.globals: list[Variable]

# If true, throw an error if a `global` variables value is not provided. Defaults to True.
CompileOptions.error_on_missing_globals: bool

# If true, throw an error if a value is provided for a `global` variable that does not exist. Defaults to True.
CompileOptions.error_on_unused_globals: bool

# If true, pre-computes certain operations during compilation when possible. Defaults to True.
CompileOptions.reduce_graph: bool
```
```python
# Example
opts = mxslc.CompileOptions()
opts.output_file = pathlib.Path("example.mtlx")
opts.version = "1.39.4"
opts.func_name = "main"
opts.func_args = [1.0, (1, 0, 0)]
opts.globals = [mxslc.Variable("metalness", 1.0), mxslc.Variable("roughness", 0.2)];
opts.reduce_graph = False
```

### `mxslc.Decompiler`

#### Constructors

```python
Decompiler.__init__(
    source: str | pathlib.Path
)
```

#### Methods

```
Decompiler.decompile_document() -> str
Decompiler.decompile_node(node_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_def(node_def_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_graph(node_graph_name: str, with_dependencies: bool = False) -> str
```

### Functions

### `mxslc.compile_*_to_*`

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
```python
# Example
opts = mxslc.CompileOptions()
opts.reduce_graph = False
mtlx = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", opts)

src_path = pathlib.Path("example.mtlx")
dst_path = mxslc.compile_file_to_file(src_path)
```

### `mxslc.decompile_*_to_*`

```
mxslc.decompile_string_to_string(source: str) -> str

mxslc.decompile_file_to_string(src_path: pathlib.Path) -> str

mxslc.decompile_string_to_file(source: str, dst_path: pathlib.Path) -> pathlib.Path

mxslc.decompile_file_to_file(src_path: pathlib.Path) -> pathlib.Path
mxslc.decompile_file_to_file(src_path: pathlib.Path, dst_path: pathlib.Path) -> pathlib.Path
```

# Decompiler

The decompiler is a tool that can be used to decompile MaterialX files into ShadingLanguageX source files.
It is available as part of the `mxslc` standalone executable (see [Getting Started](#decompile)) as well as the C++ and Python 
APIs (see [API Documentation](#api-documentation)).

## C++

### Example 1

```c++
#include <filesystem>
#include <decompile.h>

std::filesystem::path src_path{"example1.mtlx"};
std::string result = mxslc::decompile_to_string(src_path);
std::cout << result << std::endl;
```

_example1.mtlx_:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
    <normal name="node1" type="vector3" />
    <viewdirection name="node2" type="vector3" />
    <dotproduct name="node3" type="float">
        <input name="in1" type="vector3" nodename="node1" />
        <input name="in2" type="vector3" nodename="node2" />
    </dotproduct>
</materialx>
```

_Output_:
```
vector3 node1 = normal();
vector3 node2 = viewdirection();
float node3 = dotproduct(in1 = node1, in2 = node2, );
```

### Example 2

```c++
#include <filesystem>
#include <Decompiler.h>

std::filesystem::path src_path{"example2.mtlx"};
mxslc::Decompiler decompiler{src_path};
std::string result = decompiler.decompile_node("node2", /*with_dependencies*/true);
std::cout << result << std::endl;
```

_example2.mtlx_:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_add_one" node="add_one">
    <output name="out" type="float" />
    <input name="base" type="float" value="0" />
  </nodedef>
  <nodegraph name="NG_add_one" nodedef="ND_add_one">
    <add name="node1" type="float">
      <input name="in1" type="float" interfacename="base" />
      <input name="in2" type="float" value="1" />
    </add>
    <output name="out" type="float" nodename="node1" />
  </nodegraph>
  <nodedef name="ND_add_two" node="add_two">
    <output name="out" type="float" />
    <input name="base" type="float" value="0" />
  </nodedef>
  <nodegraph name="NG_add_two" nodedef="ND_add_two">
    <add name="node1" type="float">
      <input name="in1" type="float" interfacename="base" />
      <input name="in2" type="float" value="2" />
    </add>
    <output name="out" type="float" nodename="node1" />
  </nodegraph>
  <add_one name="node1" type="float">
    <input name="base" type="float" value="11" />
  </add_one>
  <add_two name="node2" type="float">
    <input name="base" type="float" value="12" />
  </add_two>
</materialx>
```

_Output_:
```
float add_two(float base = float{0}, )
{
    return node1;
}

float node2 = add_two(base = float{12}, );
```

## Python

### Example 1

```python
from pathlib import Path
import mxslc

src_path = Path("example1.mtlx")
result = mxslc.decompile_file_to_string(src_path)
print(result)
```

_example1.mtlx_:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
    <normal name="node1" type="vector3" />
    <viewdirection name="node2" type="vector3" />
    <dotproduct name="node3" type="float">
        <input name="in1" type="vector3" nodename="node1" />
        <input name="in2" type="vector3" nodename="node2" />
    </dotproduct>
</materialx>
```

_Output_:
```
vector3 node1 = normal();
vector3 node2 = viewdirection();
float node3 = dotproduct(in1 = node1, in2 = node2, );
```

### Example 2

```python
from pathlib import Path
import mxslc

src_path = Path("example2.mtlx")
decompiler = mxslc.Decompiler(src_path)
result = decompiler.decompile_node("node2", with_dependencies=True)
print(result)
```

_example2.mtlx_:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_add_one" node="add_one">
    <output name="out" type="float" />
    <input name="base" type="float" value="0" />
  </nodedef>
  <nodegraph name="NG_add_one" nodedef="ND_add_one">
    <add name="node1" type="float">
      <input name="in1" type="float" interfacename="base" />
      <input name="in2" type="float" value="1" />
    </add>
    <output name="out" type="float" nodename="node1" />
  </nodegraph>
  <nodedef name="ND_add_two" node="add_two">
    <output name="out" type="float" />
    <input name="base" type="float" value="0" />
  </nodedef>
  <nodegraph name="NG_add_two" nodedef="ND_add_two">
    <add name="node1" type="float">
      <input name="in1" type="float" interfacename="base" />
      <input name="in2" type="float" value="2" />
    </add>
    <output name="out" type="float" nodename="node1" />
  </nodegraph>
  <add_one name="node1" type="float">
    <input name="base" type="float" value="11" />
  </add_one>
  <add_two name="node2" type="float">
    <input name="base" type="float" value="12" />
  </add_two>
</materialx>
```

_Output_:
```
float add_two(float base = float{0}, )
{
    return node1;
}

float node2 = add_two(base = float{12}, );
```

# MaterialX Libraries

## `libraries` Folder

In order to use standard MaterialX nodes, `mxslc` needs access to [MaterialX libraries](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/libraries).
It looks for them next to the executable, in the `libraries` folder, followed by the name of the MaterialX version the library belongs to,
for example, `/libraries/1.39.5`.

By default, the `libraries` folder found [here](https://github.com/jakethorn/ShadingLanguageX/tree/main/mxslc%2B%2B/libraries) is copied next to the executable during the build. Make sure to always move the libraries
when moving the executable. Similarly, the `libraries` folder is included as part of the module when building the Python bindings.

## Adding Support for a MaterialX Version

Adding support for a MaterialX version is trivial. Simply download the version release from the MaterialX github release page,
rename the `libraries` folder to the version number, and copy it into the `libraries` folder next to the executable or Python module.
If you'd rather not edit the Python module directly, `mxslc` will also look for the `libraries` folder in the current working directory
as well as next to the current source file being compiled.
