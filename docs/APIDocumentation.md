<h1 align="center">API Documentation</h1>

# Table of Contents

1. [Compile Functions](#compile-functions)
2. [Compile Options](#compile-options)
3. [Macro](#macro)
4. [Variable](#variable)
5. [Decompile Functions](#decompile-functions)
6. [Reflection](#reflection)

---

# Compile Functions

The `compile` functions provide users with the functionality to compile ShadingLanguageX (.mxsl) files into MaterialX 
(.mtlx) files.

## Python

### compile_string_to_string

Compile a string containing the ShadingLanguageX source code and return the compiled MaterialX as a string. An optional 
[CompileOptions](#compile-options) object can be passed in to configure the compilation process.

```python
mxslc.compile_string_to_string(source: str) -> str
mxslc.compile_string_to_string(source: str, opts: mxslc.CompileOptions) -> str
```

### compile_file_to_string

Compile a ShadingLanguageX source file provided by `src_path` and return the compiled MaterialX as a string. An optional
[CompileOptions](#compile-options) object can be passed in to configure the compilation process.

```python
mxslc.compile_file_to_string(src_path: str | pathlib.Path) -> str
mxslc.compile_file_to_string(src_path: str | pathlib.Path, opts: mxslc.CompileOptions) -> str
```

### compile_string_to_file

Compile a string containing the ShadingLanguageX source code and save the compiled MaterialX as a file. The output file path
can be given either by the `dst_path` parameter or using `output_file` field of the optional [CompileOptions](#compile-options).

```python
mxslc.compile_string_to_file(source: str, opts: mxslc.CompileOptions) -> pathlib.Path
mxslc.compile_string_to_file(source: str, dst_path: str | pathlib.Path) -> pathlib.Path
```

### compile_file_to_file

Compile a ShadingLanguageX source file provided by `src_path` and save the compiled MaterialX as a file. The output file path
will be the same as the `src_path` with the extension changed to .mtlx, or can be given either by the `dst_path` parameter or using `output_file` field of the optional [CompileOptions](#compile-options).

```python
mxslc.compile_file_to_file(src_path: str | pathlib.Path) -> pathlib.Path
mxslc.compile_file_to_file(src_path: str | pathlib.Path, opts: mxslc.CompileOptions) -> pathlib.Path
mxslc.compile_file_to_file(src_path: str | pathlib.Path, dst_path: str | pathlib.Path) -> pathlib.Path
```

### Python Example

```python
import mxslc

mtlx = mxslc.compile_string_to_string('float f = 1 + randomfloat();')
mtlx = mxslc.compile_file_to_string('example.mxsl', mxslc.CompileOptions(func_name="main"))
mtlx_path = mxslc.compile_string_to_file('float f = 1 + randomfloat();', 'example.mtlx')
mtlx_path = mxslc.compile_file_to_file('example.mxsl')
```

## C++

### compile_to_document

Compile the source code given by `source` or `src_path`. The result is returned as a MaterialX::DocumentPtr.
An optional [CompileOptions](#compile-options) object can be passed in to configure the compilation process.

```c++
MaterialX::DocumentPtr compile_to_document(const string& source);
MaterialX::DocumentPtr compile_to_document(const string& source, const CompileOptions& opts);
MaterialX::DocumentPtr compile_to_document(const fs::path& src_path);
MaterialX::DocumentPtr compile_to_document(const fs::path& src_path, const CompileOptions& opts);
```

### compile_to_string

Compile the source code given by `source` or `src_path`. The result is returned as a string.
An optional [CompileOptions](#compile-options) object can be passed in to configure the compilation process.

```c++
string compile_to_string(const string& source);
string compile_to_string(const string& source, const CompileOptions& opts);
string compile_to_string(const fs::path& src_path);
string compile_to_string(const fs::path& src_path, const CompileOptions& opts);
```

### compile_to_file

Compile the source code given by `source` or `src_path`. The result is saved to a file.
An optional [CompileOptions](#compile-options) object can be passed in to configure the compilation process.

```c++
fs::path compile_to_file(const string& source, const CompileOptions& opts);
fs::path compile_to_file(const string& source, const fs::path& dst_path);
fs::path compile_to_file(const fs::path& src_path);
fs::path compile_to_file(const fs::path& src_path, const CompileOptions& opts);
fs::path compile_to_file(const fs::path& src_path, const fs::path& dst_path);
```

### C++ Example

```c++
#include "compile.h"
#include "CompileOptions.h"

// 1
MaterialX::DocumentPtr doc = mxslc::compile_to_document("float f = 1 + randomfloat();");

// 2
mxslc::CompileOptions opts;
opts.add_macro("USE_DISPLACEMENT", "1");
opts.add_macro("DISPLACEMENT_SCALE", "0.2");
opts.reduce_graph = false;

string code = mxslc::compile_to_string(fs::path{"disp_example.mxsl"}, opts);

// 3
fs::path mtlx_path = mxslc::compile_to_file(fs::path{"mxsl/example.mxsl"}, fs::path{"mtlx/example.mtlx"});
```

---

# Compile Options

## Python

### Constructor

```python
CompileOptions.__init__(
    output_file: str | pathlib.Path | None = None,
    version: str = "1.39.5",
    macros: list[str | tuple[str, str] | Macro] = [],
    search_directories: list[str | pathlib.Path] = [],
    includes: list[str | pathlib.Path] = [],
    libraries: list[str | pathlib.Path] = [],
    globals: dict[str, Any] = {},
    error_on_missing_globals: bool = True,
    error_on_unused_globals: bool = True,
    func_name: str | None = None,
    func_args: list[Any] = [],
    reduce_graph: bool = True,
    validate_graph: bool = True,
    sources: dict[str, str] = {},
    default_function_behaviour: FunctionBehaviour = FunctionBehaviour.NodeDef
)
```

### Fields

```python
CompileOptions.output_file: str | pathlib.Path | None
CompileOptions.version: str
CompileOptions.macros: list[str | tuple[str, str] | Macro]
CompileOptions.search_directories: list[str | pathlib.Path]
CompileOptions.includes: list[str | pathlib.Path]
CompileOptions.libraries: list[str | pathlib.Path]
CompileOptions.globals: dict[str, Any]
CompileOptions.error_on_missing_globals: bool
CompileOptions.error_on_unused_globals: bool
CompileOptions.func_name: str | None
CompileOptions.func_args: list[Any]
CompileOptions.reduce_graph: bool
CompileOptions.validate_graph: bool
CompileOptions.sources: dict[str, str]
CompileOptions.default_function_behaviour: FunctionBehaviour
```

`globals` maps global variable names to their values, and `func_args` holds the entry function's arguments in order.
Each value can be a `bool`, `int`, `float`, `str`, `pathlib.Path`, MaterialX vector, colour or matrix (e.g.,
`mx.Color3`), [Variable](#variable), or a list of these, e.g., for a struct.

`default_function_behaviour` is how functions compile when they're defined without a `nodedef`, `nodegraph`,
`inline` or `comptime` modifier: `mxslc.FunctionBehaviour.NodeDef` (the default), `NodeGraph`, `Inline` or `Comptime`.

### Methods

```python
CompileOptions.add_source(name: str, contents: str) -> None
CompileOptions.remove_source(name: str) -> None
CompileOptions.clear_sources() -> None
CompileOptions.has_source(name: str) -> bool
CompileOptions.get_source(name: str) -> str
```

`sources` holds in-memory files keyed by the path as written in an `#include` or `#library` directive. They are checked
before the file system, so a multi-file project can be compiled without writing it to disk.

### Python Example

```python
import mxslc
import MaterialX as mx

opts = mxslc.CompileOptions(
    output_file = "mtlx/example.mtlx",
    version = "1.39.4",
    macros = [],
    search_directories = ["..", "../mxsl_utils/"],
    includes = ["glsl_macros.mxsl"],
    libraries = [],
    globals = {"bg_color": mx.Color3(0.5, 0.2, 0.8)},
    error_on_missing_globals = False,
    error_on_unused_globals = True,
    func_name = "main",
    func_args = [],
    reduce_graph = True,
    validate_graph = True,
    sources = {},
    default_function_behaviour = mxslc.FunctionBehaviour.Inline
)

mtlx = mxslc.compile_file_to_string("mxsl/example.mxsl", opts)
```

## C++

```c++
// How a function compiles: the nodedef, nodegraph, inline and comptime function modifiers.
enum class FunctionBehaviour
{
    NodeDef,
    NodeGraph,
    Inline,
    Comptime
};

struct CompileOptions
{
    // ctor
    CompileOptions() = default;

    // public fields
    optional<fs::path> output_file;

    string version{"1.39.5"};
    bool reduce_graph{true};
    bool validate_graph{true};
    bool decompile_hints{false};

    // used by functions defined without a nodedef, nodegraph, inline or comptime modifier
    FunctionBehaviour default_function_behaviour{FunctionBehaviour::NodeDef};

    vector<fs::path> includes;
    vector<fs::path> libraries;

    bool error_on_missing_globals{true};
    bool error_on_unused_globals{true};

    optional<string> func_name;

    // search directory methods
    void add_search_directory(fs::path dir);
    void add_default_search_directories(const optional<fs::path>& src_path = std::nullopt);
    void set_current_working_directory(fs::path dir);
    void clear_search_directories();
    vector<fs::path> search_directories() const;

    // in-memory source methods (checked before the file system by #include and #library)
    void add_source(string name, const string& contents);
    void set_sources(unordered_map<string, string> sources);
    void remove_source(const string& name);
    void clear_sources();
    const string& get_source(const string& name) const;
    bool has_source(const string& name) const;
    const unordered_map<string, string>& sources() const;

    // macro methods
    void add_macro(Macro macro);
    void add_macro(string macro);
    void add_macro(string name, const string& body);
    void remove_macro(const string& name);
    void clear_macros();
    const Macro& get_macro(const string& name) const;
    bool has_macro(const string& name) const;
    const unordered_map<string, Macro>& macros() const;

    // global methods
    void add_global(string name, VarPtr value);
    void add_global(string name, Primitive value);
    void set_globals(unordered_map<string, VarPtr> globals);
    void set_globals(const unordered_map<string, Primitive>& globals);
    void clear_globals();
    const VarPtr& get_global(const string& name) const;
    bool has_global(const string& name) const;
    const unordered_map<string, VarPtr>& globals() const;

    // entry function argument methods
    void add_entry_function_argument(VarPtr value);
    void add_entry_function_argument(Primitive value);
    void set_entry_function_arguments(vector<VarPtr> args);
    void set_entry_function_arguments(const vector<Primitive>& args);
    void clear_entry_function_arguments();
    const vector<VarPtr>& entry_function_arguments() const;

    // debug mode (interactive debugger, -d/--debug on the command line)
    void set_debug_mode(bool debug_mode);
    bool debug_mode() const;

    // utility methods
    bool has_output_file() const;
    bool has_entry_function() const;
};
```

### C++ Example

```c++
mxslc::CompileOptions opts;
opts.output_file = "example.mtlx";
opts.version = "1.39.4";
opts.add_macro("DEBUG");
opts.add_macro("PI", "3.14159");
opts.add_search_directory("C:/Users/jaket/mtlx_libs");
opts.set_globals({{"roughness", 0.2f}, {"thin_film", true}});
opts.error_on_missing_globals = false;
opts.reduce_graph = false;

mxslc::compile_to_file(fs::path{"example.mxsl"}, opts);
```

---

# Macro

## Python

### Constructor

```python
Macro.__init__(name: str)
Macro.__init__(name: str, body: str)
```

### Python Example

```python
import mxslc
import MaterialX as mx

code = """
#ifdef OPEN_PBR
surfaceshader s = open_pbr_surface(base_color=randomcolor());
#else
surfaceshader s = standard_surface(base_color=randomcolor());
#endif

material m = surfacematerial(s);
"""

surface_type_macro = mxslc.Macro("OPEN_PBR")
opts = mxslc.CompileOptions(macros=[surface_type_macro])

mtlx = mxslc.compile_string_to_string(code, opts)
```

## C++

```c++
class Macro
{
public:
    explicit Macro(string name);
    Macro(string name, const string& body);
    Macro(string name, vector<Token> body);

    const string& name() const;
    const vector<Token>& body() const;

    bool operator==(const Macro& other) const noexcept;
    bool operator==(const string& other) const noexcept;
    bool operator==(const Token& other) const noexcept;

    bool operator!=(const Macro& other) const noexcept;
    bool operator!=(const string& other) const noexcept;
    bool operator!=(const Token& other) const noexcept;
};
```

### C++ Example

```c++
#include "preprocess/Macro.h"
#include "CompileOptions.h"

mxslc::Macro use_disp_macro{"USE_DISPLACEMENT"};
mxslc::Macro disp_scale_macro{"DISPLACEMENT_SCALE", "0.2"};

mxslc::CompileOptions opts;
opts.add_macro(use_disp_macro);
opts.add_macro(disp_scale_macro);
```

---

# Variable

## Python

### Constructor

```python
# Variable does not have a constructor
```

### Functions

```python
create_variable(value: Any | list[Any]) -> Variable
```

### Fields

```python
Variable.value: Any | list[Any]
```

### Python Example

```python
import mxslc
import MaterialX as mx

code = """
global {vec3 pos, vec3 dir} ray;
global color3 color;

// more code...
"""

ray = mxslc.create_variable([
    mx.Vector3(0, 0, 0), # pos
    mx.Vector3(0, 0, -1) # dir
])

opts = mxslc.CompileOptions(globals={"ray": ray, "color": mx.Color3(1, 0, 0)})
mtlx = mxslc.compile_string_to_string(code, opts)
```

## C++

```c++
// Variable does not have a constructor
```

### Functions

```c++
VarPtr create_variable(ModifierList mods, TypePtr type, const vector<VarPtr>& children);
VarPtr create_variable(ModifierList mods, TypePtr type, const vector<Primitive>& children);
VarPtr create_variable(ModifierList mods, TypePtr type, ValuePtr value);
VarPtr create_variable(ModifierList mods, TypePtr type, const VarPtr& value);
VarPtr create_variable(ModifierList mods, ValuePtr value);
VarPtr create_variable(ModifierList mods, Primitive value);

VarPtr create_variable(TypePtr type, const vector<VarPtr>& children);
VarPtr create_variable(TypePtr type, ValuePtr value);
VarPtr create_variable(TypePtr type, const VarPtr& value);

VarPtr create_variable(const vector<VarPtr>& children);
VarPtr create_variable(const vector<Primitive>& children);
VarPtr create_variable(ValuePtr value);
VarPtr create_variable(Primitive value);
VarPtr create_variable(const VarPtr& value);

VarPtr create_variable(ModifierList mods, TypePtr type);
VarPtr create_variable(TypePtr type);
```

---

# Decompile Functions

The `decompile` functions provide users with the functionality to decompile MaterialX (.mtlx) files into
ShadingLanguageX (.mxsl) source code.

## Python

### decompile_string_to_string

Decompile a string containing the MaterialX source and return the ShadingLanguageX source code as a string.

```python
mxslc.decompile_string_to_string(source: str) -> str
```

### decompile_file_to_string

Decompile a MaterialX file provided by `src_path` and return the ShadingLanguageX source code as a string.

```python
mxslc.decompile_file_to_string(src_path: str | pathlib.Path) -> str
```

### decompile_string_to_file

Decompile a string containing the MaterialX source and save the ShadingLanguageX source code to the file given by
`dst_path`.

```python
mxslc.decompile_string_to_file(source: str, dst_path: str | pathlib.Path) -> pathlib.Path
```

### decompile_file_to_file

Decompile a MaterialX file provided by `src_path` and save the ShadingLanguageX source code as a file. The output file
path will be the same as the `src_path` with the extension changed to .mxsl, or can be given by the `dst_path`
parameter.

```python
mxslc.decompile_file_to_file(src_path: str | pathlib.Path) -> pathlib.Path
mxslc.decompile_file_to_file(src_path: str | pathlib.Path, dst_path: str | pathlib.Path) -> pathlib.Path
```

### Decompiler

Decompile a whole MaterialX document, or individual Nodes, NodeDefs and NodeGraphs from it.

```python
Decompiler.__init__(source: str)
Decompiler.__init__(src_path: str | pathlib.Path)
```

A `str` is read as MaterialX source if it starts with an element, e.g., `<?xml` or `<materialx`, otherwise as the
path of a file, so `mxslc.Decompiler(mtlx)` and `mxslc.Decompiler("example.mtlx")` both work.

```python
Decompiler.decompile_document() -> str
Decompiler.decompile_node(node_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_def(node_def_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_graph(node_graph_name: str, with_dependencies: bool = False) -> str
```

The `with_dependencies` argument tells the decompiler to include all Nodes, NodeDefs and NodeGraphs used by
the given element or an element preceding it in the graph.

### Python Example

```python
import mxslc

slx = mxslc.decompile_file_to_string("example.mtlx")
slx_path = mxslc.decompile_file_to_file("example.mtlx")  # writes example.mxsl

mtlx = mxslc.compile_string_to_string("material m = surfacematerial(standard_surface(specular_roughness = 0.25));")
slx = mxslc.decompile_string_to_string(mtlx)

decompiler = mxslc.Decompiler(source=mtlx)
print(decompiler.decompile_node("m", with_dependencies=True))
# material m = surfacematerial(standard_surface(specular_roughness = 0.25));
```

## C++

The decompile functions and the `Decompiler` class are in the `mxslc::decompile` namespace.

### decompile_to_string

Decompile the MaterialX given by `src_path`, `source` or `document`. The result is returned as a string.

```c++
string decompile_to_string(const fs::path& src_path);
string decompile_to_string(const string& source);
string decompile_to_string(const MaterialX::DocumentPtr& document);
```

### decompile_to_file

Decompile the MaterialX given by `src_path`, `source` or `document`. The result is saved to the file given by
`dst_path`. When decompiling `src_path` without a `dst_path`, the output file path will be the same as the `src_path`
with the extension changed to .mxsl.

```c++
fs::path decompile_to_file(const fs::path& src_path, const optional<fs::path>& dst_path = std::nullopt);
fs::path decompile_to_file(const fs::path& src_path, const fs::path& dst_path);
fs::path decompile_to_file(const string& source, const fs::path& dst_path);
fs::path decompile_to_file(const MaterialX::DocumentPtr& document, const fs::path& dst_path);
```

### Decompiler

Decompile a whole MaterialX document, or individual Nodes, NodeDefs and NodeGraphs from it.

```c++
class Decompiler
{
public:
    explicit Decompiler(const fs::path& src_path);
    explicit Decompiler(const string& source);
    explicit Decompiler(const MaterialX::DocumentPtr& document);

    string decompile_document();
    string decompile_node(const string& node_name, bool with_dependencies = false);
    string decompile_node(const MaterialX::NodePtr& node, bool with_dependencies = false);
    string decompile_node_def(const string& node_def_name, bool with_dependencies = false);
    string decompile_node_def(const MaterialX::NodeDefPtr& node_def, bool with_dependencies = false);
    string decompile_node_graph(const string& node_graph_name, bool with_dependencies = false);
    string decompile_node_graph(const MaterialX::NodeGraphPtr& node_graph, bool with_dependencies = false);
};
```

A `string` is read as MaterialX source if it starts with an element, e.g., `<?xml` or `<materialx`, otherwise as
the path of a file.

The `with_dependencies` argument tells the decompiler to include all Nodes, NodeDefs and NodeGraphs used by
the given element or an element preceding it in the graph.

### C++ Example

```c++
#include "decompile/decompile.h"
#include "decompile/Decompiler.h"

// 1
string code = mxslc::decompile::decompile_to_string(fs::path{"example.mtlx"});

// 2
fs::path mxsl_path = mxslc::decompile::decompile_to_file(fs::path{"mtlx/example.mtlx"}, fs::path{"mxsl/example.mxsl"});

// 3
mxslc::decompile::Decompiler decompiler{fs::path{"example.mtlx"}};
string material_code = decompiler.decompile_node("m", true);
```

---

# Reflection

The `reflect` functions run ShadingLanguageX source code and return a `Reflection` of the program, which lists the
`global` variables it declares, in declaration order. They let a tool find out which globals a program has, e.g., to
show a parameter for each one like in the [Houdini SLX plugin](https://github.com/jakethorn/ShadingLanguageX/blob/main/tools/houdini/README.md#globals)

The source is run as it is when compiling, with two differences: missing and unused globals aren't errors, and debug
mode is off.

## Python

### reflect_from_string

Run a string containing the ShadingLanguageX source code and return its `Reflection`. An optional
[CompileOptions](#compile-options) object can be passed in to configure the run, including the values of globals.

```python
mxslc.reflect_from_string(source: str) -> mxslc.Reflection
mxslc.reflect_from_string(source: str, opts: mxslc.CompileOptions) -> mxslc.Reflection
```

### reflect_from_file

Run a ShadingLanguageX source file provided by `src_path` and return its `Reflection`. An optional
[CompileOptions](#compile-options) object can be passed in to configure the run, including the values of globals.

```python
mxslc.reflect_from_file(src_path: str | pathlib.Path) -> mxslc.Reflection
mxslc.reflect_from_file(src_path: str | pathlib.Path, opts: mxslc.CompileOptions) -> mxslc.Reflection
```

### Reflection

```python
Reflection.globals: list[mxslc.GlobalInfo]
```

`globals` is read-only. A `Reflection` is only returned by the `reflect` functions; it has no constructor.

### GlobalInfo

```python
GlobalInfo.name: str
GlobalInfo.type: str
GlobalInfo.default_value: Any
```

The fields are read-only. `type` is the variable's type name, e.g., `"float"` or `"color3"`. `default_value` is the
variable's value in [CompileOptions](#compile-options) `globals`, if it has one. Otherwise, it is the initializer's
value, or the type's default if there's no initializer. It is `None` if the initializer isn't known at compile time,
e.g., `geompropvalue<float>("x")`.

### Python Example

```python
import mxslc

code = """
global float roughness = 0.25;
global color3 tint;
global float u = texcoord<vec2>().x;
"""

reflection = mxslc.reflect_from_string(code)
for g in reflection.globals:
    print(g.name, g.type, g.default_value)

# roughness float 0.25
# tint color3 0, 0, 0
# u float None

opts = mxslc.CompileOptions(globals={"roughness": 0.5})
print(mxslc.reflect_from_string(code, opts).globals[0].default_value)

# 0.5

opts = mxslc.CompileOptions(macros=["OPEN_PBR"])
reflection = mxslc.reflect_from_file("example.mxsl", opts)
```

## C++

The `reflect` functions, `Reflection` and `GlobalInfo` are in the `mxslc::reflection` namespace.

### reflect

Run the source code given by `source` or `src_path` and return its `Reflection`. An optional
[CompileOptions](#compile-options) object can be passed in to configure the run, including the values of globals.

```c++
Reflection reflect(const string& source);
Reflection reflect(const string& source, const CompileOptions& opts);
Reflection reflect(const fs::path& src_path);
Reflection reflect(const fs::path& src_path, const CompileOptions& opts);
```

### Reflection

```c++
class Reflection
{
public:
    const vector<GlobalInfo>& globals() const;
};
```

### GlobalInfo

```c++
struct GlobalInfo
{
    string name;
    TypePtr type;
    VarPtr default_value;
};
```

`default_value` is set as it is in [Python](#globalinfo). `default_value->is_compile_time()` is false if the
initializer isn't known at compile time.

### C++ Example

```c++
#include <iostream>

#include "CompileOptions.h"
#include "reflection/reflect.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"

mxslc::CompileOptions opts;
opts.add_macro("OPEN_PBR");

mxslc::reflection::Reflection reflection = mxslc::reflection::reflect(fs::path{"example.mxsl"}, opts);
for (const mxslc::reflection::GlobalInfo& global : reflection.globals())
{
    std::cout << global.name << " " << global.type->to_string();
    if (not global.default_value->is_compile_time())
        std::cout << " (initializer not known at compile time)";
    std::cout << "\n";
}
```
