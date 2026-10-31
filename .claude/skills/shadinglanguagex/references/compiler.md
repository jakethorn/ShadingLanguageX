# Using the compiler (mxslc++)

The compiler is written in C++ and has Python bindings (`pip install mxslcxx`, then `import mxslc`) and
JavaScript/WASM bindings. The package name is `mxslcxx` but the module name is `mxslc`. It runs on
Linux, macOS and Windows with Python 3.9+.

Web tools that embed the compiler:

- the official converter, *SLX to MTLX Converter* (https://jakethorn.github.io/ShadingLanguageX/),
- *MaterialX Playground*, an SLX editor with a live MaterialX view and decompilation,
- *ShadingLanguageX Playground*, which has a project gallery.

## Command line

```text
mxslc [compile|decompile] <input-file> [options]
mxslc @options.rsp
```

| Option | Meaning |
|---|---|
| *(action)* `compile` (default) / `decompile` | Compile `.mxsl` to `.mtlx`, or decompile `.mtlx` to `.mxsl` |
| `-o, --output-file PATH` | Output path. Defaults to the input path with its extension swapped |
| `-v, --version VERSION` | Target MaterialX version (default `1.39.5`). `1.38` means 1.38.10 and `1.39` means 1.39.5. `#version` in the source overrides it |
| `-f, --func NAME` | Entry function, called after the file has executed |
| `-a, --args V...` | Entry-function arguments, consumed until the next option |
| `-g, --globals NAME VALUE ...` | Values for `global` variables. Repeatable, and takes pairs until the next option |
| `--missing-globals-ok` | A `global` without a value uses its initialiser or the type's default |
| `--unused-globals-ok` | Allow globals that are provided but never declared or used |
| `--no-reduce-graph` | Turn off compile-time folding (named constructors and `comptime` still fold) |
| `--no-validate-graph` | Skip `MaterialX::Document::validate()` |
| `-d, --debug` | Interactive step debugger (see below) |
| `--decompile-hints` | Developer option: adds decompiler hint attributes to the output and defines the macro `__DC_HINTS__` |
| `-h, --help` | Print help. It also prints a harmless "Invalid input file path: -h" |

**CLI values** for `-a` and `-g` are parsed as literals: `true`/`false`, integers, floats (`-1.1` is a
value, not an option), and **double-quoted strings**. In a shell, quote the quotes:
`-a '"brick.png"'` or `-g space '"world"'`. An unquoted word is an error. Only bool, int, float and
string values can be passed this way. Vectors, colours, matrices and structs need the API. A string
converts to `filename`, and an int to float.

The CLI can't define macros, add search directories or include files. Use `#define`, `#include` and
`#library` in the source, or the API.

**Response files** (`mxslc @file.rsp`): whitespace- or newline-separated arguments, with `#` starting a
comment, and quotes kept as written (so `"world"` stays a string literal). The input path is resolved
relative to the `.rsp` file's directory first. Arguments after `@file` are ignored.

```text
# shader.rsp
shader.mxsl -o out/shader.mtlx -v 1.39.5
-f main -a 0.5 "brick.png" true
-g roughness 0.2 tint_strength 1
--no-reduce-graph
```

**Errors**: a compile error prints `<file>, line N: <message>`. The CLI aborts through an uncaught
exception, so stderr shows `terminate called after throwing an instance of 'mxslc::CompileError'` and the
exit code is non-zero. A `[DEBUG] Loaded MaterialX version ...` line on stdout is normal.

**Debugger** (`-d`): before each top-level statement of the main file, the terminal shows three columns:
the variables in scope with their values, the source (with the current line), and the XML so far
(changed lines highlighted). Use it to see which nodes each statement creates.

## Python API

```python
import mxslc
import MaterialX as mx  # only needed for vector/color/matrix values

mtlx: str = mxslc.compile_string_to_string(src)                 # optional opts
mtlx: str = mxslc.compile_file_to_string("a.mxsl", opts)
path = mxslc.compile_file_to_file("a.mxsl")                     # -> a.mtlx; or (src, dst) or (src, opts)
path = mxslc.compile_string_to_file(src, "out.mtlx")            # or (src, opts) with opts.output_file set
gs = mxslc.get_globals_from_file("a.mxsl", opts)                # also _from_string; each has .name, .type ("color3"),
                                                                # .value (initializer or type default; None if runtime)

slx: str = mxslc.decompile_file_to_string("a.mtlx")
slx: str = mxslc.decompile_string_to_string(xml)
path = mxslc.decompile_file_to_file("a.mtlx")                   # -> a.mxsl; or (src, dst)
path = mxslc.decompile_string_to_file(xml, "a.mxsl")

d = mxslc.Decompiler(pathlib.Path("a.mtlx"))                    # a str is ALSO treated as a file path (bindings bug);
                                                                # for XML text use decompile_string_to_string
d.decompile_document(); d.decompile_node("y", with_dependencies=True)
d.decompile_node_def("ND_foo"); d.decompile_node_graph("NG_foo", with_dependencies=False)
```

`CompileOptions` constructor keywords, all optional, are also readable and writable attributes:

| Field | Default | Notes |
|---|---|---|
| `output_file` | `None` | used by `*_to_file` |
| `version` | `"1.39.5"` | MaterialX library version |
| `macros` | `[]` | items: `"NAME"`, `("NAME", "body")` or `mxslc.Macro("NAME", "body")` |
| `search_directories` | `[]` | extra directories for `#include`/`#library` and `libraries/` |
| `includes` | `[]` | files included before the main source, as if `#include`d at the top |
| `libraries` | `[]` | `.mtlx` files whose NodeDefs become callable, as with `#library` |
| `globals` | `{}` | `{"name": value}` |
| `error_on_missing_globals` | `True` | |
| `error_on_unused_globals` | `True` | |
| `func_name` | `None` | entry function |
| `func_args` | `[]` | entry-function arguments |
| `reduce_graph` | `True` | |
| `validate_graph` | `True` | |
| `sources` | `{}` | in-memory files `{"path/as/written.mxsl": "code"}`, checked before the file system. Methods: `add_source`, `remove_source`, `clear_sources`, `has_source`, `get_source` |

**Value conversion** for `globals` and `func_args`:

- `bool`, `int`, `float` and `str` map to bool, integer, float and string. A string can fill a
  `filename`.
- `os.PathLike` maps to filename.
- `mx.Vector2/3/4`, `mx.Color3/4` and `mx.Matrix33/44` map to the matching type.
- A plain `list`/`tuple` maps to an **unnamed struct** of the converted elements, so
  `{"light": [mx.Vector3(0, 10, 0), mx.Vector3(0, -1, 0), "world"]}` fills
  `global {vec3 pos, vec3 dir, string space} light;`, matching fields in order. A list of numbers
  (ints or floats) converts to a vector or colour of the same size, so `{"c": [1, 0.5, 0]}` fills a
  `global color3 c`.
- `mxslc.create_variable(value)` builds the same thing explicitly, and `.value` reads it back.
- **Known issue:** building a struct value from a Python list, whether in `CompileOptions(globals=...)`,
  `func_args` or `create_variable`, raises `Runtime not created` if nothing has been compiled yet in the
  process. Work around it by compiling once first with `mxslc.compile_string_to_string("")`, or by
  passing `mx.Vector3`, `mx.Color3` and similar instead of lists.

```python
opts = mxslc.CompileOptions(
    globals={"tint": mx.Color3(1, 0.2, 0.2), "rough": 0.3},
    macros=["USE_RAIN", ("OCTAVES", "4")],
    func_name="main", func_args=[0.5, "brick.png"],
    sources={"utils.mxsl": "inline float sq(float x) { return x * x; }"},
)
mtlx = mxslc.compile_string_to_string('#include "utils.mxsl"\n...', opts)
```

There is also a small CLI, `python -m mxslc compile in.mxsl [-o out.mtlx]`, along with
`python -m mxslc decompile in.mtlx [-o out.mxsl]` and `python -m mxslc version`. It takes no other
options.

## C++ API

Link `mxslc_lib` and include `include/`. Everything lives in `namespace mxslc`.

```cpp
#include "compile.h"           // compile_to_document / compile_to_string / compile_to_file
#include "CompileOptions.h"
#include "decompile/decompile.h"  // decompile_to_string / decompile_to_file
#include "decompile/Decompiler.h"

mxslc::CompileOptions opts;
opts.version = "1.39.5";
opts.reduce_graph = false;
opts.add_macro("USE_DISP");             // or add_macro("SCALE", "0.2")
opts.add_search_directory("shaders/lib");
opts.add_source("utils.mxsl", code);
opts.add_global("roughness", 0.2f);     // Primitive, or VarPtr via create_variable(...)
opts.func_name = "main";
opts.add_entry_function_argument(1.0f);
opts.output_file = "out.mtlx";

MaterialX::DocumentPtr doc = mxslc::compile_to_document(fs::path{"a.mxsl"}, opts);
std::string xml = mxslc::compile_to_string(std::string{"float f = 1.0;"});   // string = source code
fs::path out = mxslc::compile_to_file(fs::path{"a.mxsl"}, fs::path{"a.mtlx"});
```

A `std::string` argument is source code and an `fs::path` is a file. Errors throw `mxslc::CompileError`,
and validation failures throw `MaterialXValidateError`. `Decompiler` has the methods
`decompile_document()`, `decompile_node(name, with_dependencies)`, `decompile_node_def(...)` and
`decompile_node_graph(...)`, and can be constructed from a path, an XML string or a `DocumentPtr`.

## JavaScript / WASM

```js
import Mxslc from './JsMxslc.js';
const mx = await Mxslc();
const opts = new mx.CompileOptions();
opts.version = '1.39.5';
opts.reduceGraph = true;               // also errorOnMissingGlobals, errorOnUnusedGlobals
opts.addSource('utils.mxsl', code);    // also clearSources, addSearchDirectory, clearSearchDirectories, searchDirectories()
const mtlx = mx.compileSlxToMtlx(src, opts);     // throws Error with name 'CompileError'
const slx = mx.decompileMtlxToSlx(mtlx);
const names = mx.getMtlxDefinitionNames();       // node categories in the loaded library
opts.delete();
```

The JS API can't pass globals or an entry function.

## MaterialX versions and library lookup

- The compiler needs `libraries/stdlib.mxsl` (the SLX prelude) and `libraries/<version>/` (a copy of
  MaterialX's `libraries` folder). Builds ship versions 1.38.10, 1.39.4 and 1.39.5.
- It looks for `libraries/` in these places, in order: the main source's directory (MaterialX libraries
  only, not `stdlib.mxsl`), the `search_directories` you added, the current working directory, the Python
  module directory, and the executable directory. To add a version, copy a MaterialX release's
  `libraries` folder to `libraries/<x.y.z>` in any of those places.
- A `libraries/` folder in the working directory **overrides the bundled one**. Running an installed
  compiler from inside an mxslc++ checkout picks up that checkout's `stdlib.mxsl`. If it doesn't match
  the binary, you get errors like `stdlib.mxsl, line 434: Unexpected token: {`.
- The output's `<materialx version>` is major.minor. Validation runs only for 1.39 documents; older ones
  print *Document version (1.38) is too old to be validated*.
- Libraries are cached between compiles in the same process.

## Decompiler

`mxslc decompile a.mtlx -o a.mxsl`, or the API calls above, turns MaterialX back into readable SLX.
It reconstructs operators (`add` becomes `+`), swizzles (`p.zx`), constructors, if-expressions,
switch-expressions, `geomprop` variables, functions from NodeDef/NodeGraph pairs (marked `[[nodedef]]`),
nodegraph and parameterless functions, attributes, and destructured multi-output nodes. Its output
compiles again:

```text
{float outx, float outy} var_2 = separate2(texcoord() * 10.0);
float seed = floor(var_2.outx) + floor(var_2.outy) * 10.0;
[[nodedef]]
float add_one(float x)
{
    return x + 1.0;
}
```

Use it to learn the SLX form of an existing graph, or to port `.mtlx` files. It decompiles only what is in
the document: it can't recover loops or source-level names of folded values, and nodegraph functions
with nonlocals are a known weak spot. `with_dependencies=True` also emits every element the requested
element depends on.

## Common errors and fixes

| Message (abridged) | Cause | Fix |
|---|---|---|
| `No function could be found with the name 'vec3'` | `vec3(...)` call syntax | `vec3{...}` |
| `No function could be found with the name 'pow'` (also `mod`, `abs`, `length`, `lerp`, `log`, `cross`) | GLSL names | see the mapping table in stdlib.md |
| `The function signature '(color3 or float) image(...)' is ambiguous` | return-type overload in an untyped context | `color3 c = image(...);` or `image<color3>(...)` |
| `The function signature 'vector3 __vector3__(1.0, 2.0)' is ambiguous` | wrong constructor arity | give every component or a valid vector/scalar mix |
| `Cannot assign value to non-mutable variable: x` | missing `mutable` (variable, param, loop var, struct, out target) | declare `mutable` |
| `Variable is not available at compile-time` | runtime value in an `if` statement, loop range or `comptime` context, or a non-inline function param used that way | if-expression, or `inline` function, or compile-time value |
| `Cannot assign a non-compile-time value to a comptime variable` | node assigned to `comptime` var or param | remove `comptime` or pass a constant |
| `Missing else branch in if-expression` | `T x = if (c) { a };` in a definition | add `else`, or define first then `x = if (c) { a };` |
| `Unexpected token: &&` / `\|\|` / `Scanning error ... character: ?` | C operators | `&`, `\|`, if-expression |
| `Unexpected token: =` in `for (int i = 0:3)` | old loop syntax | `for (int i from 0 to 3)` |
| `Unexpected token: =` in a switch | `default=x` | `default: x` |
| `Cannot assign an expression of type integer to a variable or parameter of type float` | int variable into a float | `float{i}` |
| `Variable of type 'vector3' does not have a port or valid swizzle with the name: r` | `rgba` on a vector, or `xyzw` on a colour | use the matching letter set, or convert |
| `Invalid swizzle: rg` | 2-channel colour swizzle | `vec2{c.r, c.g}`, or use a color3 swizzle |
| `Temporary variable being passed to ref or out parameter` | expression passed to out/ref | pass a mutable variable |
| `Cannot access nonlocal variables in parameterless function` | `=>` function touching outer vars | pass values as parameters (normal function) |
| `Arguments cannot be passed to nodegraph functions` | `f(1)` on `[[nodegraph]]` | set defaults, or use a normal function |
| `Missing global variable: x` / `Unused global variable: x` | globals mismatch | pass `-g`, or use `--missing-globals-ok` / `--unused-globals-ok` |
| `Non-void function 'f' did not return a value` | missing return on some path | return on every compile-time path |
| `No function could be found with the name 'g'` for a function defined later | used before definition | move the definition up |
| `Type 'Point' not defined` | type used before `using`/`class` | define first |
| `Variable already defined: x` | duplicate definition (also across `#include`s) | rename, or assign instead |
| `File x could not be found. Searched paths: ...` | include or library path | check the search order in language.md §17 |
| `MaterialX version 2.70.3 libraries could not be found` | unknown version | use a bundled version, or add libraries |
| `Switch case index 100 is out of range ...` / `Switch expression has multiple default cases` | switch limits | indices 0-99, a single `default:` |
| validation error dump (`MaterialXValidateError`) | the produced document is invalid MaterialX | inspect the XML; `--no-validate-graph` to bypass |
| `Runtime not created` (Python) | a list/struct value built before any compile | run `mxslc.compile_string_to_string("")` first, or use `mx.*` types |
| `stdlib.mxsl, line N: ...` | a different version's `libraries/stdlib.mxsl` was found first (often in the working directory) | run from another directory, or fix the search paths |
