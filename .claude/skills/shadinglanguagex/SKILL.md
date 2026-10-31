---
name: shadinglanguagex
description: Write, debug, explain and compile ShadingLanguageX (SLX), the C-like shading language in `.mxsl` files that the mxslc++ compiler turns into MaterialX (`.mtlx`) node graphs. Use for any task involving .mxsl source, MaterialX materials written as code, porting GLSL/Shadertoy/OSL snippets to SLX, running the compiler (`mxslc` CLI, Python `import mxslc` from `pip install mxslcxx`, C++, JS/WASM), compile options (globals, entry functions, macros, reduce_graph, MaterialX versions), decompiling .mtlx back to SLX, or explaining which MaterialX nodes an SLX construct produces.
---

# ShadingLanguageX (SLX)

SLX is a small C-like language whose programs compile to MaterialX documents. Everything here was
verified against the mxslc++ compiler (v0.3.x, MaterialX 1.39.5 libraries). Where the official docs and
the compiler disagree, this skill follows the compiler.

## Mental model: the compiler *runs* your program

An SLX file is executed **once, top to bottom, by the compiler**. Every value is either:

- **compile-time**: literals, `const`/`comptime` values, globals, and anything folded from them, or
- **runtime**: the output of a MaterialX node (`texcoord()`, `image(...)`, `position()`, ...).

An operation on a runtime value appends a node to the graph; an operation on compile-time values is folded
into a constant (the `reduce_graph` option, on by default). The output is a static dataflow graph, so:

- `for` loops are **unrolled**; their ranges must be compile-time.
- `if (...) { stmts }` **statements** need compile-time conditions. Runtime choices use
  **if-expressions** (`if (c) { a } else { b }`) or **switch-expressions**, which become
  `ifequal`/`switch` nodes. **Both branches are always built.**
- Variables are names bound to values. They are **immutable unless declared `mutable`**. Reassigning just
  rebinds the name to a new node.
- User functions become a custom **NodeDef + NodeGraph** by default; `inline` expands them at the call site.
  Inside a non-inline function the parameters are runtime interface inputs, so they can't drive loops or
  `if` statements.
- Structs and classes exist only at compile time. Each field is its own value, and fields are flattened
  into ports (`p__x`, `p__y`) when a struct crosses a function boundary.
- Every MaterialX node is a built-in function named after its category (`image`, `mix`,
  `standard_surface`, ...), with the node's input names as parameter names.
- Runtime nodes are never removed. A compile-time value that nothing consumes emits **no** node.

## Minimal material

```slx
float scale = 10.0;
float u, v = texcoord() * scale;              // a vec2 destructures into floats (separate2)
float seed = floor(u) + floor(v) * scale;
color3 c = randomcolor(seed);

surfaceshader surf = standard_surface(base_color = c, specular_roughness = 0.4);
material mat = surfacematerial(surf);          // build materials at top level (or inside inline functions)
```

Compile it and look at the output, every time:

```bash
mxslc shader.mxsl -o shader.mtlx
```

```bash
python -c "import mxslc; print(mxslc.compile_file_to_string('shader.mxsl'))"
```

Install with `pip install mxslcxx` and `import mxslc`. The compiler's error messages are precise and list
candidate signatures. Read them, fix the code, and recompile. To print values at compile time, use
`print expr;`.

## Syntax at a glance

```text
// comments           /* block comments */
bool int float vec2 vec3 vec4 color3 color4 mat3 mat4 string filename          (long forms: boolean integer vector2 ...)
surfaceshader displacementshader volumeshader lightshader material BSDF EDF VDF  auto
float x = 1.0;  mutable float y;  const float PI = 3.14159;  comptime int N = 4;  global float rough;  geomprop float h = 0.5;
float a, int b, string c = 1.0, 2, "hi";      float s, t = texcoord();       a, b = b, a;   (multi-assign, both mutable)
vec3{1, 2, 3}  color3{0.5}  vec4{v3, 1.0}  float{i}  bool{1}      // NAMED CONSTRUCTORS USE BRACES
{1.0, "world"}                                 // unnamed constructor / tuple / struct literal
v.xy  c.rgb  v[0]  s.field  s[0]  t[-1]  surf.base_color = c;  surf.coat = null;   // swizzle, index, field, port
+ - * / % ^(power; xor on bools)  == != < <= > >=  & | !  |x|(absval)  ++ -- += -= *= /= %= ^= &= |=
if (c) { a } else if (d) { b } else { e }      // expression; `x = if (c) { a };` keeps x when false
switch (i) { a, b, case 5: c, case <=3: d, default: e }    // expression, indices 0-99
if (COMPILE_TIME) { ... } else { ... }         // statement (compile-time condition only)
for (int i from 0 to 9) { }   for (float f from 0.0:0.5:2.0) { }   for (auto x from {1, "a"}) { }
float f(float x, float k = 2.0) { return x * k; }      f(1.0, k = 3.0);    // named args after positional
inline / comptime / [[nodegraph]] / nodedef / default   T f<float, vec3>(T x) { }   f<vec3>(v);
void g(out float o, ref vec3 r) { }   g(float o, my_mutable_vec);    // declare out-variables in the call
float viewing => { return dotproduct(normal(), viewdirection()); }   // parameterless fn, use as `viewing`
using Ray = {vec3 orig, vec3 dir};   class Sphere { vec3 c; float r; Sphere(vec3 c) { this.c = c; this.r = 1.0; } float area() { return 12.566 * r * r; } }
Ray __add__(Ray a, Ray b) { ... }             // operator overloading by function name
@doc "text"  @file.colorspace "srgb_texture"  @@madewith "slx"      // node / port / document attributes
print expr, expr;   typeof(x).name   typeof(x).str   default   default(T)   null
#include "f.mxsl"  #library "defs.mtlx"  #version 1.38.10  #define N 4  #ifdef/#ifndef/#if/#elif/#elifdef/#elifndef/#else/#endif
```

## Rules that trip people up, especially coming from GLSL or HLSL

1. **Constructors use braces**: `vec3{1, 0, 0}`, never `vec3(1, 0, 0)` (error: *No function could be found with
   the name 'vec3'*). Argument counts must match an overload exactly: `vec3{x, y}` is an error, and so is
   `vec4{x, v2}`. Valid forms: `T{}`, `T{scalar}` (splat or convert), all components, or vector/scalar mixes
   such as `vec3{v2, z}`, `vec3{x, v2}`, `vec4{v3, w}`, `vec4{v2, v2}`, `vec4{x, v2, w}`. `vec4{}` and
   `color4{}` are all zeros. Matrices have no named constructor: use `default(mat3)` or `creatematrix(...)`.
2. **Immutable by default.** Add `mutable` to any variable, parameter or struct field you reassign. That
   includes loop accumulators, out-argument targets and swizzle or index writes. Parameters are immutable
   too; a `mutable` parameter is a local copy.
3. **Logic operators are `&` and `|`.** `&&`, `||` and `?:` don't exist. `&` and `|` have the **same**
   precedence, evaluated left to right, so `a | b & c` means `(a | b) & c`. Use parentheses.
4. **Unary minus binds tighter than `^`.** `-x ^ 2` means `(-x) ^ 2`, and `^` is left-associative. Write
   `-(x ^ 2)` if that's what you mean.
5. **Ternary relations** like `0 < x < 1` aren't supported outside `#if`. Write `0 < x & x < 1`.
6. **`==` and `!=` only work on bool, int and float.** Comparing vectors needs a component-wise expression.
   Relational operators only work on int and float.
7. **Only literals convert implicitly**: int literal to float or bool, and string literal to filename. An
   `int` variable needs `float{i}`. A float variable can't go into an int slot without `int{f}` (which
   rounds).
8. **Return-type overloading.** Many nodes (`image`, `texcoord`, `geompropvalue`, `noise2d`, `constant`, ...)
   are overloaded on their return type. Give them a typed destination, `color3 c = image("a.png");`, or
   an explicit template, `image<color3>("a.png") * tint`. Writing `auto x = texcoord();` or using
   `image(...) * tint` directly is ambiguous.
9. **GLSL names differ.** Use `power` (not pow), `modulo` (mod), `absval` or `|x|` (abs), `magnitude`
   (length), `dotproduct` (dot), `crossproduct` (cross), `ln` (log), `mix` (lerp), `clamp(x, 0, 1)`
   (saturate; MaterialX `saturate` is a colour-saturation op), and `fract`/`floor`/`ceil`/`round`/`sign`.
   `dot(x)` is MaterialX's pass-through node, not a dot product. **`mix(fg, bg, t)` = `fg*t + bg*(1-t)`**,
   so GLSL `mix(a, b, t)` becomes `mix(b, a, t)`. **`smoothstep(in, low, high)`** takes `in` first and
   returns 1 whenever `in >= high`, so reversed edges (`e0 > e1`) need a helper (see stdlib.md). For
   GLSL `step(e, x)`, write `float{x >= e}`.
10. **Swizzles**: use `xyzw` on vectors and `rgba` on colours, and don't mix the two sets. A swizzle result
    has 1 to 4 channels, but a colour swizzle can't have 2 channels because there is no `color2`.
    Assigning to a swizzle can't repeat a channel (`v.xx = ...` is an error).
11. **Loops**: the only form is `for (type name from range)`. Ranges are **inclusive** and ascending
    (`0 to 3` is 0,1,2,3, and `5 to 1` is empty), and their bounds must be compile-time. There is no
    `break`, `while`, or C-style `for (;;)`. A plain vector isn't iterable, so index it instead.
12. **Runtime branching**: use if-expressions. An `if` *statement* on a runtime value fails with
    *Variable is not available at compile-time*. The `else` branch is mandatory except in a plain
    assignment `x = if (c) { a };`.
13. **Declare before use**: functions, types and variables must be defined above their first use, and
    there is no recursion. Strings have no escape sequences. There are no hex literals; floats accept
    `1.`, `.5`, `2e3` and `1.5f`.
14. **Keywords** can't be used as identifiers: `if else for from to return null ref out const mutable
    consteval global geomprop nodegraph nodedef inline default comptime using class this uniform varying
    namespace print typeof break switch case true false`. Avoid `T`, `auto` and `void` as names too. Don't
    use `__` in identifiers, because the compiler uses it internally. MaterialX inputs named `default` or
    `geomprop` still work as named args (`image("a.png", default = color3{1})`).
15. **Classes** have fields, methods, constructors and `this`, but **no inheritance and no class templates**.
    Field initialisers (`float r = 1.0;` inside a class) are **silently ignored**, so set defaults in a
    constructor.
16. **Macros** support only object-like and flag forms: `#define PI 3.14159` and `#define USE_X`. There
    are no function-like macros. `#if` takes literal expressions after macro expansion; an undefined or
    empty macro in `#if` is an error, so use `#ifdef`. `#include` only accepts `.mxsl` files. Use
    `#library` for `.mtlx` NodeDefs, and remember that the generated document references those nodes
    without containing them.
17. **Switch default case**: write `default: value`. `default=value` is a syntax error.

## Picking a function kind

| Declaration | Result in MaterialX | Use when |
|---|---|---|
| `float f(float x) {...}` (or `nodedef`) | `ND_f` NodeDef + `NG_f` NodeGraph, `<f>` node per call; emitted even if never called; templated → one per type (`ND_f_vector3`) | reusable custom node; params are runtime |
| `inline float f(...)` | body expanded at each call site | helpers, GLSL ports, anything whose params drive loops/ifs, the entry function, anything building a `material` |
| `comptime float f(...)` | inline, with everything folded as far as possible (runtime args are fine, they just stay nodes) | constant math you want folded even under `--no-reduce-graph` |
| `[[nodegraph]] float f(float a = 1)` | standalone `NG_f` NodeGraph; every param needs a default; call with no args (`f()`, `f(o = out float x)`) | a NodeGraph without NodeDef |
| `float f => {...}` (parameterless) | `NG_f` NodeGraph wired by `nodegraph="NG_f"`; can't touch nonlocals; not void | constant sub-graphs |

Functions without one of these modifiers use the `default_function_behaviour` compile option, which is
`NodeDef` unless set to `NodeGraph`, `Inline` or `Comptime` (`--default-function-behaviour inline` on the CLI).

A non-inline function that reads or writes outer variables gets hidden ports (`nonlocal_in__x` and
`nonlocal_out__x`), and out parameters become `outparam__name` outputs. Prefer `inline` unless the
user wants custom NodeDefs. The documentation's own examples write entry functions as
`inline void main() {...}`.

## Workflow for writing SLX

1. Find the exact node signatures: input names and order, types, and template forms, in
   [references/stdlib.md](references/stdlib.md). Every line there has been compile-checked.
2. Write the code. Use typed declarations rather than `auto` around overloaded nodes, add `mutable`
   wherever a value is reassigned, and keep anything that has to be compile-time (loop bounds, `if`
   statements, indices used for writes) compile-time.
3. Compile. Fix errors using the candidate signatures the compiler prints. Check the `.mtlx` for node
   count and structure.
4. For variants, use `global` variables (`-g name value`), macros (API only, or `#define`), or an entry
   function (`-f main -a ...`).

## Known compiler quirks (v0.3.x) and workarounds

- Matrices can't be multiplied or divided by a float (`m * 2.0`, `2.0 * m`, `m / 2.0`).
- A negative index on a **vector** (`v[-1]`) crashes the compiler. Negative indices only work on
  structs and tuples.
- Recursion crashes instead of reporting an error. `auto` doesn't work as a return type.
- `#library` paths are resolved relative to the **main** file, even inside included files. A file with
  the same relative name in the process's working directory takes priority over one next to the
  including file.
- The compositing node `out(...)` can't be called because `out` is a keyword.
- A non-inline function that returns `default(T)` for a shader type (for example `volumeshader`) emits an
  invalid `constant` node and fails validation. Make it `inline`.
- In Python, `mxslc.Decompiler("...")` always treats a `str` as a file path. For XML text use
  `decompile_string_to_string`.

## References (load as needed)

- [references/language.md](references/language.md): the complete language reference covering lexing,
  types, conversions, operators, variables, structs, classes, functions, templates, out/ref, switch,
  attributes, scope and the preprocessor.
- [references/materialx-lowering.md](references/materialx-lowering.md): exactly which nodes each
  construct produces, how nodes are named, what compile-time folding covers, and performance tips.
- [references/compiler.md](references/compiler.md): every CLI flag, response files, the Python, C++ and
  JS APIs, `CompileOptions`, search paths, MaterialX versions, the decompiler, the debugger, and common
  error messages with their fixes.
- [references/stdlib.md](references/stdlib.md): SLX signatures for every MaterialX 1.39.5 node,
  calling conventions, SLX-only extensions, and a GLSL-to-SLX mapping. The file is about 50 KB, so
  read the header and then search for the node you need (for example `grep -n " image[<(]"`) instead of
  loading the whole listing.
- [references/examples.md](references/examples.md): complete shaders that compile, covering patterns,
  noise, loops, structs, classes, entry functions and globals, GLSL ports, and displacement.

## Maintaining this skill

- `python scripts/check_skill.py --mxslc <path/to/mxslc> [--stdlib]` compiles every `slx` code block in
  these files, and with `--stdlib` every signature in stdlib.md. Run it after compiler changes.
- `python scripts/gen_stdlib.py <libraries/x.y.z> --update references/stdlib.md` regenerates the
  signature listing for another MaterialX version.
