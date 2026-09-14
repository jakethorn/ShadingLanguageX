<h1 align="center">API Documentation</h1>

# Table of Contents

1. [Compile Functions](#compile-functions)
2. [Compile Options](#compile-options)
3. [Macro](#macro)
4. [Variable](#variable)
5. [Decompile Functions](#decompile-functions)

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
#include "mxslc/compile.h"
#include "mxslc/CompileOptions.h"

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
    macros: list[str | tuple[str, str] | Macro] | None = None,
    search_directories: list[str | pathlib.Path] | None = None,
    includes: list[str | pathlib.Path] | None = None,
    libraries: list[str | pathlib.Path] | None = None,
    globals: dict[str, Any] | None = None,
    error_on_missing_globals: bool = True,
    error_on_unused_globals: bool = True,
    func_name: str | None = None,
    func_args: list[Any] | None = None,
    reduce_graph: bool = True,
    validate_graph: bool = True
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
```

### Python Example

```python
import mxslc
import MaterialX as mx

opts = mxslc.CompileOptions(
    output_file = "mtlx/example.mtlx",
    version = "1.39.4",
    macros = None,
    search_directories = ["..", "../mxsl_utils/"],
    includes = ["glsl_macros.mxsl"],
    libraries = None,
    globals = {"bg_color": mx.Color3(0.5, 0.2, 0.8)},
    error_on_missing_globals = False,
    error_on_unused_globals = True,
    func_name = "main",
    func_args = None,
    reduce_graph = True,
    validate_graph = True
)

mtlx = mxslc.compile_file_to_string("mxsl/example.mxsl", opts)
```

## C++

```c++
struct CompileOptions
{
    // ctor
    CompileOptions() = default;

    // public fields
    optional<fs::path> output_file;

    string version{"1.39.5"};
    bool reduce_graph{true};

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

    // macro methods
    void add_macro(Macro macro);
    void add_macro(string macro);
    void add_macro(string name, string body);
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

mxslc::compile_to_file("example.mxsl", opts);
```

---

# Macro

## Python

### Constructor

```python
Macro.__init__(
    name: str,
body: str | None = None
)
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
#include "mxslc/preprocess/Macro.h"
#include "mxslc/CompileOptions.h"

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

### mxslc::decompile_to_*

```c++
string decompile_to_string(const fs::path& src_path);
string decompile_to_string(const string& source);
string decompile_to_string(const MaterialX::DocumentPtr& document);

fs::path decompile_to_file(const fs::path& src_path, const optional<fs::path>& dst_path = std::nullopt);
fs::path decompile_to_file(const fs::path& src_path, const fs::path& dst_path);
fs::path decompile_to_file(const string& source, const fs::path& dst_path);
fs::path decompile_to_file(const MaterialX::DocumentPtr& document, const fs::path& dst_path);
```

### mxslc::Decompiler

```c++

namespace mxslc
{
    class Decompiler
    {
    public:
        Decompiler(const fs::path& src_path);
        Decompiler(const string& source);
        Decompiler(MaterialX::DocumentPtr document);

        string decompile_document();
        string decompile_node(const string& node_name, bool with_dependencies = false);
        string decompile_node(const MaterialX::NodePtr& node, bool with_dependencies = false);
        string decompile_node_def(const string& node_def_name, bool with_dependencies = false);
        string decompile_node_def(const MaterialX::NodeDefPtr& node_def, bool with_dependencies = false);
        string decompile_node_graph(const string& node_graph_name, bool with_dependencies = false);
        string decompile_node_graph(const MaterialX::NodeGraphPtr& node_graph, bool with_dependencies = false);
    };
}
```

The `with_dependencies` argument tells the decompiler to include all Nodes, NodeDefs and NodeGraphs used by
the given element or an element preceding it in the graph.

## Python

### `mxslc.CompileOptions`

#### Constructors

```python
CompileOptions.__init__(
    output_file: str | pathlib.Path | None = None,
    version: str = "1.39.5",
    macros: list[str | tuple[str, str] | Macro] | None = None,
    search_directories: list[str | pathlib.Path] | None = None,
    includes: list[str | pathlib.Path] | None = None,
    libraries: list[str | pathlib.Path] | None = None,
    globals: dict[str, Variable | bool | int | float | str] | None = None,
    error_on_missing_globals: bool = True,
    error_on_unused_globals: bool = True,
    func_name: str | None = None,
    func_args: list[Variable | bool | int | float | str] | None = None,
    reduce_graph: bool = True
)
```

#### Properties

```python
options.output_file: str | pathlib.Path | None
options.version: str
options.macros: list[str | tuple[str, str] | Macro]
options.search_directories: list[str | pathlib.Path]
options.includes: list[str | pathlib.Path]
options.libraries: list[str | pathlib.Path]
options.globals: dict[str, Variable | bool | int | float | str]
options.error_on_missing_globals: bool
options.error_on_unused_globals: bool
options.func_name: str | None
options.func_args: list[Variable | bool | int | float | str]
options.reduce_graph: bool
```

#### Example

```python
opts = mxslc.CompileOptions();
opts.output_file = "example.mtlx"
opts.version = "1.39.4"
opts.macros = ["DEBUG", ("PI", "3.14159")]
opts.search_directories = ["C:/Users/jaket/mtlx_libs"]
opts.globals = {"roughness": 0.2, "thin_film": True}
opts.error_on_missing_globals = False
opts.reduce_graph = False

mxslc.compile_file_to_file("example.mxsl", opts);
```

### `mxslc.compile_*_to_*`

```python
mxslc.compile_string_to_string(source: str) -> str
mxslc.compile_string_to_string(source: str, opts: mxslc.CompileOptions) -> str

mxslc.compile_file_to_string(src_path: str | pathlib.Path) -> str
mxslc.compile_file_to_string(src_path: str | pathlib.Path, opts: mxslc.CompileOptions) -> str

mxslc.compile_string_to_file(source: str, opts: mxslc.CompileOptions) -> pathlib.Path
mxslc.compile_string_to_file(source: str, dst_path: str | pathlib.Path) -> pathlib.Path

mxslc.compile_file_to_file(src_path: str | pathlib.Path) -> pathlib.Path
mxslc.compile_file_to_file(src_path: str | pathlib.Path, opts: mxslc.CompileOptions) -> pathlib.Path
mxslc.compile_file_to_file(src_path: str | pathlib.Path, dst_path: str | pathlib.Path) -> pathlib.Path
```

```python
# Example
opts = mxslc.CompileOptions()
opts.reduce_graph = False

mtlx = mxslc.compile_string_to_string("float f = 1.0 + 1.0;", opts)

src_path = pathlib.Path("example.mxsl")
dst_path = mxslc.compile_file_to_file(src_path)
```

### `mxslc.decompile_*_to_*`

```python
mxslc.decompile_string_to_string(source: str) -> str

mxslc.decompile_file_to_string(src_path: str | pathlib.Path) -> str

mxslc.decompile_string_to_file(source: str, dst_path: str | pathlib.Path) -> pathlib.Path

mxslc.decompile_file_to_file(src_path: str | pathlib.Path) -> pathlib.Path
mxslc.decompile_file_to_file(src_path: str | pathlib.Path, dst_path: str | pathlib.Path) -> pathlib.Path
```

### `mxslc.Decompiler`

#### Constructors

```python
Decompiler.__init__(
    source: str | pathlib.Path
)
```

Use `str` for inline MaterialX source text, and use `pathlib.Path` for filesystem input files.

#### Methods

```python
Decompiler.decompile_document() -> str
Decompiler.decompile_node(node_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_def(node_def_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_graph(node_graph_name: str, with_dependencies: bool = False) -> str
```
