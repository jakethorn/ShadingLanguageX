# SLX language reference

This reference was verified against the mxslc++ sources: `scan.cpp`, `parse.cpp`, `preprocess.cpp` and
`libraries/stdlib.mxsl`. Code blocks tagged `slx` compile as written. Blocks tagged `text` are grammar
sketches or deliberately invalid examples.

## Contents
1. Lexical structure
2. Program structure
3. Types
4. Conversions and constructors
5. Variables and modifiers
6. Operators and precedence
7. Access: fields, swizzles, indexing, ports
8. Ranges
9. If-expressions, if-statements, switch-expressions
10. For loops
11. Functions
12. User-defined types: structs, `using`, classes
13. Operator overloading
14. `null`, `default`, `typeof`, `print`
15. Attributes
16. Scope and name resolution
17. Preprocessor

---

## 1. Lexical structure

- Whitespace and newlines are insignificant. Comments are `// line` and `/* block */`.
- Identifiers match `[_a-zA-Z][_a-zA-Z0-9]*`. Don't use `__` in your own names, because the compiler uses
  it for generated element names.
- **Keywords** (reserved): `if else for from to return null ref out const mutable consteval global
  geomprop nodegraph nodedef inline default comptime using class this uniform varying namespace print
  typeof break switch case`, plus the literals `true false`. Also treat `T`, `auto` and `void` as
  reserved, since they are type names with special meaning. `uniform`, `varying`, `namespace` and
  `consteval` are reserved but do nothing yet.
- Literals:
  - `true`, `false`
  - int: `0`, `79`, `1000000` (decimal only, no hex)
  - float: `0.5`, `2.`, `.9`, `2.5e6`, `2.e3`, `.9e-3`, `3e1`, and an optional `f`/`F` suffix (`1.5f`)
  - string: `"..."`. There are **no escape sequences** and a string can't contain `"`.
- Symbols: `( ) { } [ ] . , : ; @ @@ ! = > < + - * / % ^ & | #`, compounds `!= == >= <= += -= *= /= %= ^=
  &= |= ++ -- => [[ ]]`. The lexer recognises `&&`, `||`, `->` and `::`, but the parser rejects them.
- A bare `break;` statement at the top level is parsed and ignored (a legacy breakpoint marker).

## 2. Program structure

A file is a list of statements executed in order. Stray `;` are allowed at the top level.

| Statement | Example |
|---|---|
| variable definition | `float a = 0.0;` |
| multi-variable definition | `float a, b = 0.0, 1.0;` |
| assignment / multi-assignment | `a = 1.0;` `a, b = 1.0, 2.0;` |
| expression statement | `standard_surface(base = 1.0);` |
| function / class / using definitions | see below |
| if statement (compile-time) | `if (N > 2) { ... }` |
| for loop | `for (int i from 0 to 3) { ... }` |
| return | `return x;` |
| print | `print a, b;` |
| document attribute | `@@madewith "slx"` |

A file needs no `main`. Top-level code builds the graph directly. An entry function is optional (see
`compiler.md`).

## 3. Types

| Type | Alias | Notes |
|---|---|---|
| `boolean` | `bool` | |
| `integer` | `int` | |
| `float` | | |
| `vector2` / `vector3` / `vector4` | `vec2` / `vec3` / `vec4` | components `xyzw` |
| `color3` / `color4` | | channels `rgba` |
| `matrix33` / `matrix44` | `mat3` / `mat4` | rows via `m[i]` |
| `string`, `filename` | | |
| `surfaceshader`, `displacementshader`, `volumeshader`, `lightshader`, `material`, `BSDF`, `EDF`, `VDF` | | closure/shader types, only produced by nodes or `default(T)` |
| `{T1 n1, T2 n2}` / `{T1, T2}` | | unnamed struct (tuple) |
| `using Name = type;` | | alias for a struct **or any type** (`using real = float;`) |
| `class Name {...}` | | named struct with methods and constructors |
| `auto` | | inferred from the initialiser. Allowed in variables, struct fields and parameters with a default; don't use it as a return type |

Arrays don't exist. Use structs or tuples, which can be indexed (see §7).

## 4. Conversions and constructors

**Implicit conversions** (assignment, argument passing, return):
- int **literal** to float or bool (`float f = 1;`, `bool b = 1;`). An int *variable* does not convert, so
  write `float{i}`.
- string **literal** to filename.
- Unnamed struct of N floats to and from `vec2/3/4` and `color3/4` with N components, in either direction:
  `vec2 v = {1.0, 2.0};`, `float x, y = v;`, `color4 c = foo_returning_4_floats();`, and passing
  `{1.0, 2.0}` to a `vec2` parameter.
- Struct to struct when the field count and types match. Field names and modifiers may differ.
- Values supplied through the API or CLI (globals, entry-function args) convert like literals, so an int
  works for a float global.

When more than one overload could accept an implicit conversion, the call is ambiguous and fails.

**Named constructors** `T{...}` (bool, int, float, vec2-4, color3/4) are compile-time (`comptime`)
functions defined in `stdlib.mxsl`:

| Form | Meaning | Node, if the arg is runtime |
|---|---|---|
| `T{}` | zero/false; **`vec4{}` and `color4{}` are (0,0,0,0)** | none (constant) |
| `T{x}` with `x` of type T | identity | none |
| `T{x}` with another type | MaterialX `convert`: a scalar splats (`color3{0.5}`), and a bool or int becomes 0/1 (`float{flag}`). Vectors and colours of different sizes drop trailing channels (`vec2{v3}`), or pad with 0 and **w/alpha = 1** (`vec4{v3}` = `(x, y, z, 1)`). `vec3` and `color3` convert channel for channel | `convert` |
| `bool{float}` | `x != 0` | `ifequal` + `not` |
| `int{float}` | rounds | `round` |
| `vec2{x, y}` `vec3{x, y, z}` `vec4{x, y, z, w}` (same for colours) | build | `combine2/3/4` |
| `vec3{v2, z}` `vec3{x, v2}` `color3{rg, b}` `color3{r, gb}` | mixed | `separate2` + `combine3` |
| `vec4{v3, w}` `vec4{x, v3}` `vec4{v2, v2}` `vec4{v2, z, w}` `vec4{x, v2, w}` `vec4{x, y, v2}` `color4{c3, a}` ... | mixed | separate + combine |

Arity must match one of these. `vec3{1.0, 2.0}` and `vec4{1.0, v2}` are **errors**, because missing
components are not zero-filled. Keyword args such as `color3{g = 1.0}` don't work either. Class types
also accept `ClassName{args}`, which calls the constructor (see §12).

**Unnamed constructor** `{a, b, ...}` evaluates each element against the target type, so
`vec3 p = {1, 2, 3};`, `Point q = {1.0, 2.0};` and `{float, string} t = {1.0, "x"};` all work.
Elements are typed by context, so `{randomfloat(), 0}` passed to a `vec2` parameter works.

## 5. Variables and modifiers

```text
[modifiers] type name [= expr];
[modifiers] type a, [mods] [type] b, ... [= expr | = e1, e2, ...];
```

- Without an initialiser, a variable holds the type's default value (zero, false, `""` or an empty shader),
  except `global`, which comes from the compiler options.
- Redefining a name in the same scope is an error (*Variable already defined*). Inner scopes may shadow.

Variable modifiers. Any of them may be wrapped in `[[ ]]`, for example `[[mutable]] float x;` or
`[[inline comptime]] float f(...)`.

| Modifier | Meaning |
|---|---|
| *(none)* | immutable after definition |
| `mutable` | may be reassigned, including fields, components and swizzles |
| `const` | immutable, and overrides a `mutable` container (see §12) |
| `comptime` | the value must always be compile-time. `mutable comptime float a; a = 5;` is fine, but assigning a node is an error |
| `global` | the value is injected by the compiler (`-g name value` or `CompileOptions.globals`). The initialiser is used **only** when missing globals are allowed |
| `geomprop` | becomes `geompropvalue(geomprop="<name>", default=<initialiser>)`. Combined with `global` (`geomprop global float x;`), the global value becomes the default |

`const` and `mutable` together is an error. `geomprop` isn't allowed on parameters.

**Multi-variable definition and assignment**

```slx
float a, int b, string c = 1.0, 2, "hello";
const float d, e, f = {1.0, 2.0, 3.0};         // shared mods+type; tuple initialiser
float u, v = texcoord();                       // vec2 splits into its components (separate2)
float x, y = separate2(vec2{1, 2});            // multi-output node -> struct -> destructure
mutable float p, q = 1, 2;
p, q = q, p;                                   // multi-assignment: RHS is evaluated first
mutable float r, s, t = 0, 0, 0;
r, s, t = position() + 1.0;                    // a vec3 assigned to three floats
standard_surface(base = a + float{b} + u + x + p + r, normal = normal(c));
```

**Increment, decrement and compound assignment** are *expressions*. Their target must be mutable.
`i++` evaluates to the old value and `++i` to the new one. `a op= b` (`+= -= *= /= %= ^= &= |=`)
evaluates `b` fully, assigns, and returns the new value, so `randomfloat(seed = i += 1)` is valid.
Operands must have matching types: `int_var += 1.0` is an error.

## 6. Operators and precedence

Highest first. Binary operators are left-associative.

| Level | Operators |
|---|---|
| primary | literals, names, calls, `T{}`, `{}`, `(e)`, `\|e\|` (absval), `if`/`switch` expressions, `default`, `typeof`, `null`, `this` |
| postfix | `a.b`, `a.m()`, `a[i]` |
| increment | `++a --a a++ a--` |
| compound assignment | `a += b` and friends (the RHS is a whole expression) |
| unary | `-a +a !a` (**binds tighter than `^`**) |
| power | `a ^ b` (power for numbers, xor for bools) |
| multiplicative | `* / %` |
| additive | `+ -` |
| range | `a:b` `a:b:c` `a to b` |
| relational | `> >= < <=` (a chain `a < b < c` parses but is rejected) |
| equality | `== !=` |
| logical | `&` `\|` (**same level**, left to right) |

The logical `|` operator can't appear inside `|...|`, even in parentheses. If the expression needs one,
call `absval(...)` instead.

Type rules come from `stdlib.mxsl` overloads:
- `+ - * / % ^` work on `T op T` and `T op float` / `float op T` for float, vec2-4 and color3/4.
- Matrices (mat3/mat4) support `m + m`, `m + float`, `float + m`, `m - m`, `m - float`, `m * m` and
  `m / m`. **`m * float` isn't defined.** `float - mat3` works, but `float - mat4` fails because of a bug
  in `stdlib.mxsl`. For transforms, use `transformmatrix(v, m)`.
- int supports `+ - * / ^` but **not `%`**. `int * int`, `int / int` (floors) and `int ^ int` are
  computed through float and rounded.
- `BSDF/EDF/VDF` support `+`, plus `* float` and `* color3`.
- `< <= > >=` work on int or float only. `== !=` work on bool, int or float only. `& | !` and `^`
  (as xor) work on bool only.
- Unary `-` works on numbers and vectors (`invert(x, amount = 0)`, or `subtract(0, x)` for int).

## 7. Access: fields, swizzles, indexing, ports

**Dot operator**. What `.name` means depends on the value:
1. a **field** or **parameterless method** of a struct or class (`p.x`, `s.area`), or a method call
   `s.area()`.
2. a **swizzle** on vectors (`x y z w`) and colours (`r g b a`), 1 to 4 characters, within the vector's
   length. Vector swizzles give float or vec2-4. Colour swizzles give float, color3 or color4, so
   2-channel colour swizzles like `c.rg` are invalid. Swizzles chain (`v.xyz.xy.x`) and are assignable
   when there are no duplicate channels (`mutable vec4 v; v.wy = vec2{1, 2};`, `c.bgr = c.rgb;`).
3. a **node port** on a node-valued variable: read or write an input
   (`surfaceshader s = standard_surface(); s.base_color = c; float r = s.specular_roughness;`). Reading an
   unset port returns the NodeDef default and adds that input to the node. Assigning `null` removes the
   input. Port writes are allowed on immutable variables.
   Multi-output nodes return structs, so `separate2(v).outx` works.

**Indexing** `a[i]`:
- Struct or tuple fields by position. Negative indices count from the end (`t[-1]`), but only on
  structs, not vectors. A **compile-time** index produces no nodes. A **runtime** index works only when
  all fields share a type, and becomes a `switch` node. Writing a struct field needs a compile-time
  index.
- Vector or colour components: reads become `extract` and accept runtime indices. Writes `v[i] = x`
  rebuild the vector (separate + combine), and with a runtime index they build every permutation plus a
  `switch`, which is expensive.
- Matrix rows: `m[1]` gives a vec3 or vec4 row, and `m[1][2] = 5.0` works on a mutable matrix.

## 8. Ranges

`a to b`, `a:b` and `a:b:step` produce a **tuple** of values from `a` to `b` **inclusive**, stepping by
`step` (default 1). They work with ints or floats (`0.0:0.5:2.0` gives `{0.0, 0.5, 1.0, 1.5, 2.0}`).
Ranges only ascend: a negative step or `a > b` produces an empty tuple. All operands must be
compile-time. A range is an ordinary tuple, so `auto r = 7 to 11; r[0]` works, and `{int, int, int} t = 1 to 3;`
is valid.

## 9. If-expressions, if-statements, switch-expressions

**If-expression** (runtime select):

```text
if (cond) { expr } else if (cond2) { expr2 } else { expr3 }
```
- `cond` must be a bool. Both branches must have the same type, and that includes structs. Each branch
  holds a single expression, not statements.
- `else` is required, with one exception: when the if-expression is the whole right-hand side of a plain
  assignment (`x = if (c) { v };`), the missing branch keeps the old value of `x`. That form doesn't
  work for definitions.
- A compile-time `cond` short-circuits, so only the chosen branch is evaluated. Otherwise both branches
  are built.

**If-statement** (compile-time only):

```text
if (cond) { statements } else if (cond) { statements } else { statements }
```
`cond` must be known at compile time. A literal int is accepted as a bool, but a string is an error. It is
commonly used with globals, macros or comptime/inline parameters. `return` inside it works, and in
compile-time-unrolled loops too.

**Switch-expression** (runtime select by index):

```text
switch (which) { case-list }       // which: int or float; 1..100 cases
```
| Case | Selects |
|---|---|
| `expr` | the next positional index (counting from 0, in order of appearance) |
| `case n: expr`, `case ==n: expr` | index `n`. `n` can be an expression (`case 1+1:`) |
| `case <n:` `case <=n:` `case >n:` `case >=n:` `case !=n:` | every index 0-99 satisfying the comparison |
| `default: expr` | every index no other case selects, at most once, wherever it is written |

Later cases override earlier ones for the same index. Indices must be integers in 0-99. All cases must
have the same type, structs included. A compile-time `which` folds to the chosen case. The
`switch(...)` *node* can still be called as a function when no `{` follows:
`switch(1.0, 2.0, which = i)`.

```slx
float i = randomfloat(max = 20.0);
float b = switch (i) { case <=7: 1.0, case 5: 2.0, case 6: 3.0, default: 4.0 };
int k = geompropvalue("k", 0);
vec3 v = switch (k) { vec3{1, 0, 0}, vec3{0, 1, 0}, default: vec3{0, 0, 1} };
standard_surface(base = b, normal = v);
```

## 10. For loops

```text
for ([mods] type name from tuple-expr) { statements }
```
- Iterates over the *fields* of a compile-time-sized tuple, usually a range: `0 to 9`, `0:2:10`,
  `{1, 4, 2}`, or a mixed tuple with `auto`.
- The loop variable is immutable unless declared `mutable`, and each element must convert to its type.
  `for (int i from 1.0 to 3.0)` fails, while `for (float f from 1 to 3)` is fine.
- The body is **unrolled** once per element. Bounds can come from globals, macros, `comptime` values or
  inline-function parameters, but never from runtime nodes.
- A vector isn't a tuple, so `for (float c from vec3{...})` fails. Use `for (int i from 0 to 2) { v[i] }`
  instead.

```slx
mutable float height = 0.0;
mutable float scale = 2.0;
vec2 uv = position().xy;                        // hoist shared nodes out of the loop
for (int i from 0 to 3)
{
    float s2 = scale * scale;
    height += noise2d(texcoord = uv / s2) * s2;
    scale *= 2.0;
}
surfacematerial(standard_surface(), null, displacement(height));
```

## 11. Functions

### Definition and calls
```text
[mods] RetType name[<T1, T2, ...>](params) { statements }
[mods] RetType name[<T1, ...>] => { statements }          // parameterless
param := [@attrs] [mods] type name [= default-expr]       // mods: const mutable comptime out ref
```
- Use `void` for no return value. A non-void function must return on every path, and `return;` is only
  valid in a void function. Several `return`s are fine when they sit under compile-time conditions.
- Calls look like `name(args)`. Positional args come first, then named ones (`name = expr`) in any order.
  Putting a positional arg after a named one is an error. Defining two parameters with the same name is
  also an error.
- Default values can be any expression and are evaluated **at the call**, so they see the current values
  of outer variables. Default `null` (the default for every MaterialX input) means "leave the input
  unset".
- Functions must be declared before they are called. There is no recursion, which crashes the compiler.
  Functions may be nested inside functions.
- **Overloading** includes the return type. `vec2 f()` and `vec3 f()` can coexist, and the expected type
  at the call picks one. A `default` modifier on an overload breaks ties:
  `default vec3 foo(float in = 0.0)` beats `vec3 foo(int in = 0)` for `foo()`.

### Templates
`T name<type1, type2, ...>(T x, ...)` stamps out one function per listed type. Inside the function,
`T` is a type name (`T{1.0}`, `image<T>(f)`, `default(T)`, `typeof(T).name`). There is a single
type parameter named `T`. The type is inferred from the arguments or the expected return type, or given
explicitly: `name<vec3>(...)`.

```slx
inline T one_minus<float, vec2, vec3, color3>(T v) { return 1.0 - v; }
inline T image_mult<vec3, color3>(filename path, T mult = T{1.0})
{
    T img = image<T>(path);
    return img * mult;
}
color3 albedo = image_mult("brick.png", color3{0.8, 0.1, 0.1});
vec3 n = image_mult<vec3>("normals.png");
standard_surface(base_color = one_minus(albedo), normal = n);
```

### Parameter passing
- Parameters are **immutable** by default. A `mutable` parameter is a local copy, so the caller doesn't
  see changes.
- `out` parameters are mutable. They start at their declared default or zero, and their final value is
  written back to the caller's variable.
- `ref` is like `out` but starts with the caller's value.
- For `out` and `ref`, the argument must be a **mutable lvalue**: a variable, a mutable field, a
  component (`v[2]`) or a swizzle (`v.wy`). Temporaries are rejected. Prefixing the argument with `out`
  is optional (`f(out x)`), but doing it for a non-out parameter is an error.
- **Variable-declaration arguments** declare the target inside the call: `sincos(a, float s, float c);`
  or `f(out float o)`. Add `mutable` to modify it later (`f(mutable float o)`), or `const`.
- A `comptime` parameter requires a compile-time argument.

```slx
void sincos(float r, out float s, out float c) { s = sin(r); c = cos(r); }
inline void bump(ref vec3 p) { p = p * 2.0; }
float ang = randomfloat();
sincos(ang, float s, float c);
mutable vec3 pos = position();
bump(pos);
standard_surface(base = s + c, normal = pos);
```

### Function kinds (modifiers)
| Modifier | Effect |
|---|---|
| *(default)* / `nodedef` | NodeDef + NodeGraph. Parameters are runtime interface inputs, so they can't be used as loop bounds or `if`-statement conditions |
| `inline` | expanded at each call site. Parameters keep their compile-time-ness |
| `comptime` | inline, and everything inside is folded where possible (`power` and similar still emit nodes, see the lowering reference) |
| `nodegraph` | a NodeGraph with no NodeDef. Every in-parameter needs a default, and you can't pass arguments except out-params by name |
| `default` | tie-breaker in overload resolution |

`default` combines with any other modifier, and `inline` and `comptime` may be combined. These pairs
are errors: inline+nodegraph, inline+nodedef, comptime+nodegraph, comptime+nodedef and
nodegraph+nodedef.

### Parameterless functions
`float name => { ...; return x; }` is called by **name alone** (`name`, `name<vec3>` or `obj.name`), and
`name()` is an error. These functions are pure: reading or writing any nonlocal variable is an error.
They may call other functions that are themselves free of nonlocals, declare `global`s, be templated,
inline or class members, but not void. Non-inline ones become a standalone NodeGraph.

```slx
inline T one<vec2, vec3> => { return T{1}; }
float facing => { return dotproduct(normal("world"), viewdirection("world")); }
{vec3 n, string space} frame => { return {normal(), "world"}; }
vec3 o = one;                           // template deduced from the destination
surface_unlit(emission = facing, emission_color = color3{frame.n + o});
```

### Nonlocal access
Functions may read and write variables from enclosing scopes. Writes need the outer variable to be
mutable. A non-inline function exposes these as `nonlocal_in__x` inputs and `nonlocal_out__x` outputs.
A `nodegraph` function can't reference variables of an enclosing *function*.

## 12. User-defined types

### Unnamed structs and `using`
```text
{[mods] type [name], ...}         // either every field is named or none are
using Name = {float x, float y};
```
Fields are accessed by name or index. A struct literal is `{...}`. Assigning or passing between struct
types needs matching field count and types, while names and modifiers may differ.

**Mutability inheritance.** A field inherits mutability from its container unless it declares its own:
- `mutable {float x, const float y} a;` lets you assign `a.x` but not `a.y`.
- `{mutable float x, float y} a;` lets you assign `a.x` but not `a.y`, and assigning the whole of `a`
  is an error.
- `mutable Point p;` lets you assign the whole of `p` and any non-const field.

### Classes
```slx
class Circle
{
    vec2 center;
    float radius;

    Circle(vec2 center, float radius = 1.0)      // constructor; may be `inline`
    {
        this.center = center;
        this.radius = radius;
    }

    bool contains(vec2 p) { return distance(p, center) < radius; }   // fields usable bare or via this.
    Circle grown(float d) { Circle c = this; return Circle{c.center, c.radius + d}; }
}

Circle a = Circle{vec2{0.5}, 0.25};    // calls the constructor
Circle b = {vec2{0.2}, 0.1};           // field-order struct literal also works
bool hit = a.grown(0.05).contains(texcoord()) | b.contains(texcoord());
standard_surface(base_color = color3{float{hit}});
```
- A class body may contain only fields (single or multi definitions), methods and constructors. Fields
  can be declared anywhere in the body, but a method can only call methods declared above it.
- A trailing `;` after the closing brace is optional.
- Field initialisers are ignored, so use a constructor. There is no inheritance (`class B : A` parses
  but B doesn't get A's fields) and no class templates.
- Methods that change `this` need a `mutable` instance at the call site (`mutable Sphere s; s.grow(1.0);`).
- Parameterless methods (`vec3 n => {...}`) are accessed as `obj.n` and can't touch fields.

## 13. Operator overloading

Define free functions, never class members:

| `a+b` `__add__` | `a-b` `__sub__` | `a*b` `__mul__` | `a/b` `__div__` | `a%b` `__mod__` | `a^b` `__pow__` |
|---|---|---|---|---|---|
| `a>b` `__gt__` | `a>=b` `__ge__` | `a<b` `__lt__` | `a<=b` `__le__` | `a==b` `__eq__` | `a!=b` `__ne__` |
| `a&b` `__and__` | `a\|b` `__or__` | `!a` `__not__` | `-a` `__neg__` | `+a` `__pos__` | `\|a\|` `__abs__` |

Compound assignments use the binary overload (`p += q` calls `__add__`). Declare overloads `inline`
unless you want NodeDefs named `__add__`.

Watch out for unnamed structs and `using` aliases with 2 to 4 float fields: they convert implicitly to
vectors, so without an overload `p + q` still compiles and adds them as `vec2`. Your overloads always
win over that conversion. Class types don't convert, so they need the overload.

```slx
using V2 = {float x, float y};
inline V2 __add__(V2 a, V2 b) { return {a.x + b.x, a.y + b.y}; }
inline V2 __mul__(V2 a, float s) { return {a.x * s, a.y * s}; }
inline float __abs__(V2 a) { return sqrt(a.x * a.x + a.y * a.y); }
V2 p = {randomfloat(), 2.0};
mutable V2 q = {3.0, 4.0};
q += p * 2.0;
standard_surface(base = |q|);
```

## 14. `null`, `default`, `typeof`, `print`

- `null`: "no value". Passed or assigned to a node input, it leaves the input unset so MaterialX's
  default applies (`s.coat = null;`, `surfacematerial(surf, null, disp)`). It's useful as a parameter
  default (`float min = null`).
- `default` / `default(T)`: the default value of the inferred or given type. That's 0, false, `""`, a
  zero vector or colour, the **identity** matrix, or an empty shader or closure. It works for structs too
  (`{bool, vec4} a = default;`).
- `typeof(expr or Type)` gives `{string name, string str}`. `name` is the type name (`"vector3"`), or
  `""` for an unnamed struct. `str` is the full form (`"{integer, vector2, string}"`,
  `"Sphere{vector3 center, float radius}"`).
- `print e1, e2;` writes each value to stdout at compile time. Constants print as values (`2`, `"hi"`,
  `vec3{1.0, 2.0, 3.0}`) and runtime values print as `node (category, name)` or
  `node-output (category, name.output)`.

## 15. Attributes

```text
@name "value"            // attribute on the element the next statement/param/arg creates
@port.name "value"       // attribute on that element's input/output `port`
@@name "value"           // attribute on the <materialx> document
```
- On a **function definition**, the attribute goes on the NodeDef. `@in.doc` targets the input `in`,
  and `@out.doc` targets the return output, which is named `out`.
- On a **parameter**, written before it, the attribute goes on the NodeDef input.
- On a **variable definition, assignment, expression statement or return**, it goes on the node made by
  the right-hand side. `@file.colorspace "srgb_texture"` goes on that node's `file` input. This only
  works when the expression is a **direct node or function call** (`image(...)`, `sin(x)`,
  `standard_surface(...)`). On operator expressions such as `float y = x * 2.0;` the attribute is
  silently dropped, so write `multiply(x, 2.0)` if you need one there.
- On an **argument** (`image(@colorspace "srgb_texture" "a.png")`), it goes on that input.
- Values must be string literals. Names may contain `-` (`@Autodesk-hidden "true"`).

```slx
@@madewith "ShadingLanguageX"

@doc "albedo texture"
@file.colorspace "srgb_texture"
color3 albedo = image("albedo.png");

@nodegroup "math"
@doc "scales then offsets"
@out.doc "the result"
float mad(@doc "input" float x, float m = 2.0, float a = 1.0) { return x * m + a; }

standard_surface(base_color = albedo, base = mad(randomfloat()));
```

## 16. Scope and name resolution

- Braces create scopes (function bodies, loop bodies, if-statement blocks). Names declared inside aren't
  visible outside.
- Scopes nest like this: MaterialX libraries, then `stdlib.mxsl` extensions, then the file's global
  scope, then each block.
- Inner declarations hide outer ones of the same name. Function lookup checks the innermost scope that
  has a matching overload and then falls back outward. So defining `inline float clamp(float x)` adds an
  overload, and `clamp(x, 0.0, 1.0)` still reaches the MaterialX node.
- Assigning to an outer variable requires it to be `mutable`, and the assignment is visible after the
  call (non-inline functions do this through nonlocal ports).

## 17. Preprocessor

Directives are processed before parsing. They run line-based and expand macros in their arguments.

| Directive | Behaviour |
|---|---|
| `#include "path.mxsl"` | Paste the tokens of an SLX file. Each file is included at most once, later includes are skipped, and circular includes are an error. The path may be a macro or a string expression. `.mtlx` can't be included |
| `#library "defs.mtlx"` | Load the NodeDefs in that MaterialX file as callable functions. They are **not** copied into the output, so the renderer must load that library too |
| `#version 1.38.10` | Target MaterialX version: an unquoted number, which may come from macros (`#version MAJOR.MINOR.PATCH`). `1.38` means 1.38.10 and `1.39` means 1.39.5. It is honoured inside a taken `#if` branch |
| `#define NAME [tokens to end of line]` | Object-like or flag macro. Redefining replaces it. Expansion is recursive at use. **No function-like macros**, `#` or `##` |
| `#undef NAME` | |
| `#ifdef/#ifndef NAME`, `#if expr`, `#elif expr`, `#elifdef`, `#elifndef`, `#else`, `#endif` | Conditional compilation. `#if` evaluates literals after expansion with `+ - * / % ^ == != < <= > >= & \| !`, parentheses and chains (`#if 0.0 < X < 1.0`). There is no `defined()`, and an empty or undefined name is an error |

Predefined macros:

- `__MAIN__`, defined while preprocessing the main file.
- `__INCLUDE__`, defined inside included files.
- `__FILE__`, the current file name as a string.
- `__LINE__`, the current line number.

Macros also come from `CompileOptions.macros`, but the CLI has no `-D`. A macro replaces **every**
matching identifier token, including field and parameter names, so define GLSL-compat macros (for
example `#define mod modulo`) after code that uses those names in other roles.

Search order for a relative `#include` path:

1. in-memory sources (`CompileOptions.sources`),
2. **the path relative to the process's current working directory**, so a same-named file there wins,
3. the directory of the file containing the directive,
4. `CompileOptions.search_directories`,
5. the current working directory, then the Python module directory, then the executable directory.

`#library` paths are collected during preprocessing and resolved **afterwards, relative to the main
file's directory**, even when the directive sits in an included file in another folder. Write library
paths relative to the main file, or make them absolute.
