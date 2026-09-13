# API Documentation

## C++

### mxslc::CompileOptions

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
    const unordered_map<string, Macro>& macros() const { return macros_; }

    // global methods
    void add_global(string name, VarPtr value);
    void add_global(string name, Primitive value);
    void set_globals(unordered_map<string, VarPtr> globals);
    void set_globals(const unordered_map<string, Primitive>& globals);
    void clear_globals();
    const VarPtr& get_global(const string& name) const;
    bool has_global(const string& name) const;
    const unordered_map<string, VarPtr>& globals() const { return globals_; }

    // entry function argument methods
    void add_entry_function_argument(VarPtr value);
    void add_entry_function_argument(Primitive value);
    void set_entry_function_arguments(vector<VarPtr> args);
    void set_entry_function_arguments(const vector<Primitive>& args);
    void clear_entry_function_arguments();
    const vector<VarPtr>& entry_function_arguments() const { return func_args_; }

    // utility methods
    bool has_output_file() const { return output_file.has_value(); }
    bool has_entry_function() const { return func_name.has_value(); }
};
```

#### Example

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

### mxslc::compile_to_*

```c++
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
```

#### Example

```c++
mxslc::CompileOptions opts;
opts.reduce_graph = false;

string mtlx = mxslc::compile_to_string("float f = 1.0 + 1.0;", opts);

fs::path src_path{"example.mxsl"};
fs::path dst_path = mxslc::compile_to_file(src_path);
```

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
    macros: list[str | tuple[str, str] | Macro] = [],
    search_directories: list[str | pathlib.Path] = [],
    includes: list[str | pathlib.Path] = [],
    libraries: list[str | pathlib.Path] = [],
    globals: dict[str, Variable | bool | int | float | str] = {},
    error_on_missing_globals: bool = True,
    error_on_unused_globals: bool = True,
    func_name: str | None = None,
    func_args: list[Variable | bool | int | float | str] = [],
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
opts.macros = ["DEBUG", ["PI", "3.14159"]]
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

```python
mxslc.decompile_string_to_string(source: str) -> str

mxslc.decompile_file_to_string(src_path: pathlib.Path) -> str

mxslc.decompile_string_to_file(source: str, dst_path: pathlib.Path) -> pathlib.Path

mxslc.decompile_file_to_file(src_path: pathlib.Path) -> pathlib.Path
mxslc.decompile_file_to_file(src_path: pathlib.Path, dst_path: pathlib.Path) -> pathlib.Path
```

### `mxslc.Decompiler`

#### Constructors

```python
Decompiler.__init__(
    source: str | pathlib.Path
)
```

#### Methods

```python
Decompiler.decompile_document() -> str
Decompiler.decompile_node(node_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_def(node_def_name: str, with_dependencies: bool = False) -> str
Decompiler.decompile_node_graph(node_graph_name: str, with_dependencies: bool = False) -> str
```
