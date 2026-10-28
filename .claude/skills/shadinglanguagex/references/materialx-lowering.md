# How SLX compiles to MaterialX

All outputs below come from mxslc++ with default options unless a note says otherwise. Every value the
compiler holds is one of: a compile-time constant, a node or node output, a NodeGraph interface input
(inside functions), or a NodeGraph reference.

## Pipeline

1. **Scan** each file into tokens.
2. **Preprocess**: run directives, expand macros, splice includes. The prelude `libraries/stdlib.mxsl` is
   preprocessed too.
3. **Load libraries**: the MaterialX data libraries for the target version, every `.mtlx` under
   `libraries/<version>/`, become functions in an outer scope. `#library` files are added next.
4. **Parse and execute**: `stdlib.mxsl` runs in scope `mxsl_stdlib` (type aliases, operator and
   constructor overloads), then the user file runs in scope `global`, statement by statement.
   Executing an expression either folds it or writes nodes into the document.
5. If `func_name` is set, **invoke the entry function** with `func_args`.
6. **Check globals**: any global that was provided but never used is an error unless allowed.
7. **Validate** with `MaterialX::Document::validate()` when the document's major.minor version matches
   the MaterialX version the compiler links against (1.39). Older targets only get a warning. Then the
   XML is written. The output never includes the data libraries or `#library` NodeDefs.

## Compile-time evaluation

- With `reduce_graph = true` (the default), a node call whose inputs are all compile-time is evaluated
  by the compiler **if its category is foldable**. The foldable categories are `add`, `subtract`,
  `multiply`, `divide`, `invert`, `not`, `ifequal`, `ifgreater`, `ifgreatereq`, `constant`,
  `combine2/3/4`, `separate2/3/4`, `convert`, `extract`, `creatematrix`, `invertmatrix`, `switch`,
  `fract`, `dotproduct`, `floor`, `ceil` and `round`.
- Everything else **always emits a node**, even with constant inputs. That includes `power` (`^`),
  `modulo` (`%`), `absval` (`|x|`), `and`, `or`, `xor`, trig, `sqrt`, `clamp`, `mix`, noises and
  `smoothstep`.
- `--no-reduce-graph` (`reduce_graph = false`) disables folding for ordinary code. Named constructors
  (`vec3{...}`, `color3{...}`, `float{...}`) are `comptime`, so they **always** fold. `comptime`
  expressions, variables and functions also fold regardless of the option.
- A folded value appears as a `value="..."` attribute on whatever consumes it. A constant that is never
  consumed emits nothing, so `float x = 1.0 + 1.0;` on its own produces an empty document. Wrap a value in
  `constant(x)` with `--no-reduce-graph` if you need a node for it.
- Runtime nodes are never removed, even when unused, and there's no common-subexpression elimination.
  Calling `texcoord()` three times creates three `texcoord` nodes.

## Node naming

| Situation | Node name |
|---|---|
| `float theta = dotproduct(n, i);` | `theta`, the defining variable's name |
| intermediate or unnamed values | `var__0`, `var__1`, ... (`var__N`) |
| reassignment of a mutable `h` | `var__h__1`, `var__h__2`, ... |
| struct field definitions | `ray__orig`, `ray__dir`; `a__0`, `a__1` for unnamed fields |
| locals of inlined functions | their own names, deduplicated as `img`, `img2`, ... |
| a call to a multi-output node or function bound to a variable | the variable's name, with `type="multioutput"` and consumers using `output="..."` |

When a name is taken, MaterialX's `createValidChildName` adds a suffix.

## Operators

| SLX | Nodes |
|---|---|
| `a + b` / `a - b` / `a * b` / `a / b` | `add` / `subtract` / `multiply` / `divide`. A vector or colour operand is always put in `in1`, so `2.0 * v` becomes `multiply(v, 2)`. `float - v` and `float / v` first `convert` the float |
| `a % b` / `a ^ b` | `modulo` / `power`; `xor` for bools |
| `int * int`, `int ^ int` | `convert` both to float, apply `multiply`/`power`, then `round` |
| `int / int` | `convert`, `divide`, `floor` |
| `-a` | `invert(in = a, amount = 0)`; for int, `subtract(0, a)` |
| `+a` | nothing |
| `!a`, `a & b`, `a \| b` | `not`, `and`, `or` |
| `a > b` / `a >= b` | `ifgreater(a, b)` / `ifgreatereq(a, b)`, giving a boolean |
| `a < b` / `a <= b` | `ifgreater(b, a)` / `ifgreatereq(b, a)`, operands swapped |
| `a == b` / `a != b` | `ifequal(a, b)` / `ifequal` + `not` |
| `\|a\|` | `absval` |
| `a++`, `a += b`, ... | the corresponding `add` / `subtract` / ... node, rebinding `a` |

## Constructors, swizzles and indexing

| SLX | Nodes |
|---|---|
| `vec3{x, y, z}` (runtime parts) | `combine3` |
| `vec3{v2, z}`, `color4{c3, a}` | `separate2/3` + `combine3/4` |
| `color3{v3}`, `vec2{v3}`, `float{b}`, `color3{0.5}` | `convert` |
| `v.y` | `extract(in = v, index = 1)` |
| `v.zx`, `c.bgr` | `separateN` + `combineN` |
| `v.xy = e;` `v[0] = e;` (compile-time index) | `separateN` + `combineN` rebuild |
| `v[i]` read with runtime `i` | `extract` with `index` connected |
| `v[i] = e` with runtime `i` | `separateN`, a `combineN` for every possible index, then `switch(which = i)`. Expensive |
| `struct[i]` with runtime `i` (all fields the same type) | `switch` over the fields |
| `m[r]` | `extract` returning vec3 or vec4 |
| `float a, b = v;` | `separate2/3/4` (type `multioutput`), consumed through `output="outx"` and so on |

## Control flow

**If-expression.** `if (c) { a } else { b }` builds a boolean node for `c` and then
`ifequal(value1 = c, value2 = true, in1 = a, in2 = b)`. `else if` chains nest from the inside out. Both
branches are always evaluated, and a compile-time `c` short-circuits with no nodes.

`color3 c = if (r > 0.5) { color3{1, 0, 0} } else { color3{0, 0, 1} };` compiles to:

```xml
<ifgreater name="var__0" type="boolean">
  <input name="value1" type="float" nodename="r" />
  <input name="value2" type="float" value="0.5" />
</ifgreater>
<ifequal name="c" type="color3">
  <input name="value1" type="boolean" nodename="var__0" />
  <input name="value2" type="boolean" value="true" />
  <input name="in1" type="color3" value="1, 0, 0" />
  <input name="in2" type="color3" value="0, 0, 1" />
</ifequal>
```

**Switch-expression.** A single `switch` node carries the case values in `in1` to `in10`, with
`which` set to the index. Unselected slots between cases repeat the default case, or are left unset.
More than 10 cases build a two-level tree: an int `which` is `convert`ed to float, then `divide` by 10
and `modulo` 10 feed inner `switch` nodes, which feed an outer one. Struct-typed switches emit one
switch per field (`p__x`, `p__y`).

**For loops** are fully unrolled. Each iteration emits its own copy of the body's nodes.

**If-statements** emit nothing themselves. The chosen block executes and the other is skipped.

## Functions

### Default (NodeDef) functions
```slx
mutable float acc = 0.0;
float r = randomfloat();
float add_scaled(float x, float s = 2.0) { acc += x; return x * s + r; }
float a = add_scaled(1.0);
standard_surface(base = a + acc);
```
This produces:

- `<nodedef name="ND_add_scaled" node="add_scaled">` with output `out`, inputs `x` (value 0) and
  `s` (value 2), plus hidden ports `nonlocal_in__acc`, `nonlocal_out__acc` and `nonlocal_in__r`.
- `<nodegraph name="NG_add_scaled" nodedef="ND_add_scaled">` whose nodes read parameters through
  `interfacename="x"` and so on.
- At the call site, an `<add_scaled ... type="multioutput">` node, since it has several outputs. The
  return value is read with `output="out"` and the new `acc` with `output="nonlocal_out__acc"`.

Rules:
- The NodeDef and NodeGraph are emitted **when the function is defined, even if it's never called**.
  Templated functions emit one pair per listed type: `ND_f_float`, `ND_f_vector3`, with node categories
  `f_float` and `f_vector3`.
- The NodeDef input default is the parameter default evaluated at definition time, or zero. Default
  expressions are evaluated again at each call. With `float by = k` and a mutable `k` that has changed,
  the call node gets `by` set to the current `k`.
- Struct parameters and returns are flattened: inputs `p__x` and `p__y`, outputs `out__x` and `out__y`.
- `out` parameters become outputs `outparam__name`. `ref` parameters become an input `name` **and** an
  output `outparam__name`.
- Methods become `ND_<Class>_<method>`, with the instance's fields as inputs `this__field`. When the
  method mutates `this`, it also gets outputs `out__this__field`.
- Constructors become `ND___<Class>__` (node `__Class__`) with outputs `out__field`. Fields pass through
  `dot` nodes.
- Operator overloads that aren't `inline` become NodeDefs named `__add__` and so on. Prefer `inline`.
- Nested functions and nonlocal access are supported. Every nonlocal read and write becomes a port.

### Inline and comptime
`inline` functions add no elements. Their statements run in the caller's scope at each call, so locals
become ordinary nodes. A `comptime` function is inline with folding forced on.

### Nodegraph functions
```slx
float r = randomfloat();
[[nodegraph]]
float foo(float a = 1, float b = r) { return a + b; }
standard_surface(base = foo());
```
This produces a `<nodegraph name="NG_foo">` with graph-level `<input name="a" value="1">` and
`<input name="b" nodename="r">`, plus `<output name="out">`. The consumer connects with
`<input name="base" nodegraph="NG_foo" />`.

### Parameterless functions
`float viewing => {...}` becomes `<nodegraph name="NG_viewing">` with no inputs, and every use connects
via `nodegraph="NG_viewing"`. A struct return produces outputs `out__<field>`, which consumers select
with `output="out__n"`.

### Entry functions
With `-f main`, the named function is called once after the file runs. If it isn't `inline`, the output
contains `ND_main` and `NG_main`, and the material ends up **inside** that NodeGraph, with one `<main>`
node instance at the top level. Most renderers expect a top-level `surfacematerial`, so declare entry
functions `inline` (`inline void main(...)`).

## Other constructs

| SLX | Result |
|---|---|
| `global float x;` | no node. Its value is folded in as a constant |
| `geomprop float x = 0.5;` | `<geompropvalue name="x">` with `geomprop="x"` and `default=0.5` |
| `s.base_color = c;` | sets or replaces that input on node `s` |
| `s.base_color = null;` | removes the input |
| `@doc "..."` above a statement | `doc="..."` on the node produced by a direct call. Dropped for operator expressions such as `a * b` |
| `@file.colorspace "srgb_texture"` | `colorspace=` on the node's `file` input |
| `@@madewith "slx"` | attribute on the `<materialx>` root |
| attribute above a function | on the NodeDef. `@x.doc` goes on its input, `@out.doc` on its output |
| `#version 1.38.10` | `<materialx version="1.38">`, using that version's node set (for example, the deprecated `swizzle` node exists) |
| `print`, `typeof`, `using`, class fields | nothing. A class's non-inline methods and constructors emit NodeDefs when the class is defined |

## Performance and graph-size tips

- Hoist shared runtime nodes (`texcoord()`, `position()`, `time()`) out of loops and functions and reuse
  the variable, because there is no CSE.
- Prefer `inline` for small helpers. NodeDef functions add a NodeDef, a NodeGraph and multi-output
  plumbing, and some applications handle custom NodeDefs poorly.
- Avoid runtime-indexed component writes (`v[i] = x`) and runtime struct indexing, since they create
  permutations and `switch` nodes. Restructure them as if- or switch-expressions on whole values.
- Swizzles cost separate and combine nodes. Use `convert` (`vec2{v3}`, `color3{v4}`) to drop trailing
  components with a single node, and `extract` (`v.x`, `v[0]`) for one component.
- Branches aren't lazy. An expensive computation inside an if-expression branch always runs.
- Use `comptime` for constant math you want folded, but remember `power`, `modulo`, `absval`, `and`,
  `or` and `xor` are never folded.
