<h1 align="center">ShadingLanguageX Language Specification</h1>

<p align="center">
  <img src="https://github.com/jakethorn/ShadingLanguageX/blob/main/examples/screenshots/combined.png" />
</p>

# Table of Contents

1. [Introduction](#introduction)
2. [Shader Anatomy](#shader-anatomy)
3. [Data Types](#data-types)
4. [Expressions](#expressions)
5. [Statements](#statements)
6. [Literals](#literals)
7. [Identifiers](#identifiers)
8. [Reserved Keywords](#reserved-keywords)
9. [Whitespace](#whitespace)
10. [Comments](#comments)
11. [Operators](#operators)
12. [Variable Definition](#variable-definition)
13. [Variable Assignment](#variable-assignment)
14. [Named Constructor](#named-constructor)
15. [If Expression](#if-expression)
16. [If Statement](#if-statement)
17. [Switch Expression](#switch-expression)
18. [Compile-Time Evaluation](#compile-time-evaluation)
19. [For Loop](#for-loop)
20. [Functions](#functions)
21. [User-Defined Types](#user-defined-types-1)
22. [Unnamed Constructor](#unnamed-constructor)
23. [Operator Overloading](#operator-overloading)
24. [Print Statement](#print-statement)
25. [Attributes](#attributes)
26. [Node Constructor](#node-constructor)
27. [Null Expression](#null-expression)
28. [Standard Library](#standard-library)
29. [Scope](#scope)
30. [Preprocessor Directives](#preprocessor-directives)

# Introduction

ShadingLanguageX is a high level programming language that allows developers to write shaders that can be compiled into MaterialX (.mtlx) files.
The primary use case is to provide a method of creating MaterialX shaders that overcomes some of the weaknesses of using a node editor or the MaterialX C++ or Python API.
Node editors are very useful for getting quick feedback during shader development or when creating simple shader networks, but are difficult to use the larger the network becomes.
At the same time, the MaterialX API can be quite verbose, reducing code readability and making it difficult to quickly iterate when writing a shader, without writing your own custom wrapper around the API first.
ShadingLanguageX is a simple, yet powerful language for writing complex MaterialX shaders that aims to overcome these drawbacks.

A core aim of ShadingLanguageX is to maximize portability. At the time of writing, many renderers and frameworks only support
a subset of the MaterialX specification. To ensure that ShaderLanguageX is compatible with as many platforms as possible,
it compiles only to standard node elements. Additionally, functions compile to NodeDef/NodeGraphs
for better cohesion with the main MaterialX specification. As support for MaterialX becomes more
mature and as we continue to work on ShadingLanguageX more features will become utilised.

At the end of the day, ShadingLanguageX is based on the MaterialX specification and we've striven for equivalency as much
as possible. As such, if anything is omitted from this document, you can assume that the behaviour is the same as what is
described in the official MaterialX [specification](https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/Specification/MaterialX.Specification.md). For example, we don't specify in this document what is the return
type of a `vector3` multiplied by a `float`, as it is already described in the MaterialX Standard Node [document](https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/Specification/MaterialX.StandardNodes.md).
This is to keep this document as concise as possible as well as to reduce the chance of needing to update it in the future
due to changes to the official MaterialX documentation.

# Shader Anatomy

ShadingLanguageX shaders consist simply of a list of statements that are sequentially compiled into MaterialX nodes. For example:
```
vec3 v = viewdirection();
vec3 n = normal();
float theta = -dotproduct(v, n);
float outline = smoothstep(theta, 0.2, 0.25);
color3 c = color3{position() * outline};
surfaceshader surface = standard_surface();
surface.base_color = c;
surface.specular_roughness = 1.0;
material toon_material = surfacematerial(surface);
```
`> ./mxslc++.exe my_shader.mxsl`

compiles to:
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <viewdirection name="node1" type="vector3" />
  <normal name="node2" type="vector3" />
  <dotproduct name="node3" type="float">
    <input name="in1" type="vector3" nodename="node1" />
    <input name="in2" type="vector3" nodename="node2" />
  </dotproduct>
  <invert name="node4" type="float">
    <input name="in" type="float" nodename="node3" />
  </invert>
  <smoothstep name="node5" type="float">
    <input name="in" type="float" nodename="node4" />
    <input name="low" type="float" value="0.2" />
    <input name="high" type="float" value="0.25" />
  </smoothstep>
  <position name="node6" type="vector3" />
  <multiply name="node7" type="vector3">
    <input name="in1" type="vector3" nodename="node6" />
    <input name="in2" type="float" nodename="node5" />
  </multiply>
  <convert name="node8" type="color3">
    <input name="in" type="vector3" nodename="node7" />
  </convert>
  <standard_surface name="node9" type="surfaceshader">
    <input name="base_color" type="color3" nodename="node8" />
    <input name="specular_roughness" type="float" value="1" />
  </standard_surface>
  <surfacematerial name="node10" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="node9" />
  </surfacematerial>
</materialx>
```
![](../examples/screenshots/toon.png)

> [!WARNING]
> ### Not yet supported in mxslc++
> ShadingLanguageX shaders can also be executed with a designated entry function like in GLSL. If the shader contains
> a function called `main`, this function will be the entry into the shader. For example:
> ```
> inline void main()
> {
>     surfacematerial(
>         standard_surface(
>             base_color = color3(1.0, 0.72, 0.315),
>             specular_roughness = 0.02,
>             metalness = 1.0
>         )
>     );
> }
> ```
> `> ./mxslc++.exe my_shader.mxsl`
>
>
> Otherwise, an entry function name can be passed
> to the compiler as an additional argument. If the entry function accepts any arguments, these can also
> be passed to the compiler. For example:
> ```
> inline void my_function(float r, float g, float b, float roughness, float metalness)
> {
>     surfacematerial(
>         standard_surface(
>             base_color = color3(r, g, b),
>             specular_roughness = roughness,
>             metalness = metalness
>         )
>     );
> }
> ```
> `> ./mxslc++.exe my_shader.mxsl -m my_function -a 1.0 0.72 0.315 0.02 1.0`

![](../examples/screenshots/gold.png)

# Data Types

Supported data types match the ones found in the MaterialX specification, with the exception of arrays.

## Primitive Data Types

| Data Type            | Example                       |
|----------------------|-------------------------------|
| `boolean`            | `true` or `false`             |
| `integer`            | `79`                          |
| `float`              | `2.2`                         |
| `vector2`            | `vector2{0.0, 1.0}`           |
| `vector3`            | `vector3{0.0, 1.0, 2.0}`      |
| `vector4`            | `vector4{0.0, 1.0, 2.0, 3.0}` |
| `color3`             | `color3{1.0, 0.0, 0.0}`       |
| `color4`             | `color4{1.0, 0.0, 0.0, 1.0}`  |
| `matrix33`           | `creatematrix(...)`           |
| `matrix44`           | `creatematrix(...)`           |
| `string`             | `"tangent"`                   |
| `filename`           | `"../textures/albedo.png"`    |
| `surfaceshader`      | `standard_surface(...)`       |
| `displacementshader` | `displacement(...)`           |
| `volumeshader`       | `volume(...)`                 |
| `lightshader`        | `light(...)`                  |
| `material`           | `surfacematerial(...)`        |
| `BSDF`               | `chiang_hair_bsdf(...)`       |
| `EDF`                | `uniform_edf(...)`            |
| `VDF`                | `absorption_vdf(...)`         |

## Type Aliases

ShadingLanguageX provides the following type aliases. They are functionally equivalent to their underlying type.
In this document we will typically use the aliased version for the sake of brevity.

`boolean` ➔ `bool`  
`integer` ➔ `int`  
`vector2` ➔ `vec2`  
`vector3` ➔ `vec3`  
`vector4` ➔ `vec4`  
`matrix33` ➔ `mat3`  
`matrix44` ➔ `mat4`

> [!WARNING]
> ### Not yet supported in mxslc++
> ## Auto
>
> Variables and functions can also be declared using the `auto` keyword when the type can be inferred from the right-hand
> expression, or return expression in the case of function declarations.
>
> ### Example
>
> ```
> auto pi = 3.14;  
> auto red = color3{1.0, 0.0, 0.0};
> auto uv = texcoord<vec2>();
> 
> auto add_one(float x)
> {
>     return x + 1.0;
> }
> 
> auto randomvector<vec2, vec3, vec4>()
> {
>     float r = randomfloat();
>     return T{r};
> }
> ```

## Type Conversions

ShadingLanguageX supports a limited number of implicit conversions. Integer and string literals can be implicitly converted to floats and filenames, respectively.

Vectors can also be implicitly converted to and from unnamed structs with the appropriate number of float fields. For example:
```
vec2 uv = {1.0, 2.0};

{float, float} uv2 = uv;
float u, v = uv;

{float, float} foo(vec2 v)
{
    return v * 2.0;
}

vec2 v2 = foo({3.0, 4.0});
```

Otherwise, variables will need to be explicitly converted using a 
[Named Constructor](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#named-constructor).

```
float x = 0;                 // implicit int ➔ float
filename p = "my_image.exr"; // implicit string ➔ filename
color3 c = color3{1.0};      // explicit float ➔ color3
```

> [!TIP]
> ### New in mxslc++!
> ## User-Defined Types
>
> ShadingLanguageX supports user-defined types through
> [Unnamed Structs](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#unnamed-struct),
> [Using Statements](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#using-statement) and
> [Class Definitions](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#class-definition). For example:
> ```
> // unnamed struct
> {float, float} random_point()
> {
>     return {randomfloat(), randomfloat()};
> }
> 
> // using statement
> using Point = {float x, float y};
> 
> float dotproduct(Point a, Point b)
> {
>     return a.x * b.x + a.y * b.y;
> }
> 
> // class definition
> class Sphere
> {
>     Point center;
>     float radius;
>     
>     float area()
>     {
>         return 4.0 * 3.14 * radius * radius;
>     }
> }
> ```

# Expressions

Expressions are pieces of code that evaluate to a value, such as `1.0 + 1.0`. This document will cover each expression in detail
in its own section. The following table gives a quick overview of all the expressions supported by ShadingLanguageX.

| Expression                      | Example                              |
|---------------------------------|--------------------------------------|
| Binary Operator                 | `a + b`                              |
| Unary Operator                  | `-a`                                 |
| Ternary Relational Operator     | `x < a < y`                          |
| Field Access Operator           | `s.radius`                           |
| Swizzle Operator                | `c.rgb`                              |
| Port Access Operator            | `s.base_color`                       |
| Indexing Operator               | `a[0]`                               |
| Literal                         | `3.14`                               |
| Identifier                      | `i`                                  |
| Increment/Decrement Operator    | `i++`                                |
| Compound Assignment Operator    | `i += 2`                             |
| Grouping Expression             | `(a + b)`                            |
| If Expression                   | `if (a < b) { x } else { y }`        |
| Switch Expression               | `switch (a) { x, y, z }`             |
| Function Call                   | `my_function(a, b)`                  |
| Method Call                     | `my_obj.my_function(a, b)`           |
| Constructor Call                | `vec3{0, 1, 0}`                      |
| Unnamed Constructor Call        | `{a, b}`                             |
| Node Constructor                | `{"mix", color3: bg=a, fg=b, mix=c}` |
| Variable Declaration Expression | `foo(out float out_arg)`             |
| This                            | `this`                               |
| Null                            | `null`                               |

# Statements

Statements are pieces of code that change the state of the program, either by storing a named variable or function so it
can be accessed later or by controlling the flow of the program. Statements are typically termined with the semicolon `;`.
This document will cover each statement in detail in its own section. The following table gives a quick overview of all
the statements supported by ShadingLanguageX.

| Statement                 | Example                                     |
|---------------------------|---------------------------------------------|
| Variable Definition       | `float a = 0.0;`                            |
| Variable Assignment       | `a = 1.0;`                                  |
| Function Definition       | `{float, float} zeros() { return {0, 0}; }` |
| Multi-Variable Definition | `float a, b = zeros();`                     |
| If Statement              | `if (true) { ... } else { ... }`            |
| For Range Loop            | `for (int i = 0:9) { a++; }`                |
| For Each Loop             | `for (int i = {1, 5, 3}) { a += i; }`       |
| Expression Statement      | `standard_surface(base=1.0);`               |
| Using Statement           | `using Point = {float x, float y};`         |
| Class Statement           | `class Point { float x; float y; }`         |
| Print Statement           | `print a;`                                  |

# Literals

Literals represent the fundamental data used by the system.

| Data Type  | Examples                               |
|------------|----------------------------------------|
| `boolean`  | `true` `false`                         |
| `integer`  | `1` `79` `1000000`                      |
| `float`    | `0.5` `2.` `.9` `2.5e6` `2.e3` `.9e-3` |
| `string`   | `"tangent"` `"my_image.png"`           |

See 
[Named Constructor](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#named-constructor) 
and
[Unnamed Constructor](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#unnamed-constructor)
for information on how to initialise vectors, colors and user-defined data types.

# Identifiers

Identifiers are the names given to user-defined variables and functions so they can be accessed later in the program.
ShadingLanguageX follows the paradigm implemented in most programming languages: identifiers can contain letters, numbers and the underscore character, with the exception that the first character cannot be a number.
They cannot be the same as a ShadingLanguageX reserved keyword (see below).
It is highly recommended not to use double underscores `__` in identifiers because the compiler uses them internally for 
MaterialX element names and temporary variables/functions.

### Example

```
int i = 0;
int _i = i + 1;
vec3 UP = vec3{0.0, 1.0, 0.0};
float n_angle = dotproduct(UP, normal());
float pi2 = 3.14 * 2.0;
```

# Reserved Keywords

The following identifiers have a special meaning in ShadingLanguageX and cannot be used for user-defined variables or functions.

`if` `else` `switch` `for` `return` `true` `false` `void` `null` `T` `auto` `out` `ref` `inline` `const` `mutable` 
`global` `default` `function` `using` `class` `this` `print` `break`

### Note

ShadingLanguageX is an evolving language. Keywords might be added in each update which might cause shaders to break which
were previously working correctly. In general, try not to use identifiers that are popular keywords in similar
languages (e.g., `namespace` `struct` `typeof`) or a term that is prominantly used in the MaterialX specification (e.g., `node` `uniform` `varying`).

# Whitespace

All whitespace is treated equally in ShadingLanguageX. A single space character is the same as 10 new line characters. For example:

`float a = 1.0;`

is equivalent to:
```
float
a
=
1.0
;
```
although we do not recommend the latter for readability reasons.

# Comments

Comments in ShandingLanguageX take the sole form of: `// this is a comment`.

# Operators

## Binary Operators

| Operation | MaterialX Node(s)   |
|-----------|---------------------|
| `a + b`   | `add`               |
| `a - b`   | `subtract`          |
| `a * b`   | `multiply`          |
| `a / b`   | `divide`            |
| `a % b`   | `modulo`            |
| `a ^ b`   | `power` or `xor`    |
| `a > b`   | `ifgreater`         |
| `a >= b`  | `ifgreatereq`       |
| `a < b`   | `ifgreatereq`       |
| `a <= b`  | `ifgreater`         |
| `a == b`  | `ifequal`           |
| `a != b`  | `ifequal`+`not`     |
| `a & b`   | `and`               |
| `a \| b`  | `or`                |

## Unary Operators

| Operation | MaterialX Node(s)   |
|-----------|---------------------|
| `!a`      | `not`               |
| `-a`      | `subtract`          |
| `+a`      |                     |

### Notes

* The `^` operator compiles to a `power` node when used with numeric types and to an `xor` node when used with booleans.
* The MaterialX arithmetic nodes specify that vectors/colors must be the first input if paired with a float, however, this
  is not the case in ShadingLanguageX, a vector/color can be either the left or right value, for example, `2.0 * vec3{}`
  is equivalent to `vec3{} * 2.0`.

> [!WARNING]
> ### Not yet supported in mxslc++
> ## Ternary Relational Operator
> 
> ShadingLanguageX supports the Ternary Relational Operator, for example: `a < x < b` which is equivalent to `a < x & x < b`.
> This form can be used with any of the relational operators (i.e., `<` `<=` `>` `>=`).

## Dot Operator

The dot operator is used to either access the fields or methods of a variable, perform a swizzle operation
or access an underlying node port. Which of these operations is performed is determined by the type of the variable and its
underlying value.

> [!TIP]
> ### New in mxslc++!
> ### Field Access Operator
> 
> If the variable is a user-defined type, the dot operator can be used to access the fields or methods of that variable. For example:
> 
> ```
> class Point
> {
>     float x;
>     float y;
>     
>     float distance_to(Point other)
>     {
>         return sqrt((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
>     }
> }
> 
> Point p = {1.0, 2.0};
> Point q = {3.0, 4.0};
> float d = p.distance_to(q);
> ```

See [User-Defined Types](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#user-defined-types) for more information.

> [!WARNING]
> ### Not yet supported in mxslc++
> ### Swizzle Operator
> 
> The Swizzle Operator allows
> users to access vector components using any of `x` `y` `z` `w` or color channels using `r` `g` `b` `a` after a period `.`.
> For example, `vec3 b = a.yyz;` is equivalent to `vec3 b = vec3{a[1], a[1], a[2]};`.
> The characters `x` `y` `z` `w` can only be used to access components from a vector, it is a syntax error to use them with
> a color type variable, and vice versa for `r` `g` `b` `a`.
> Swizzles can be made from any combination of valid characters, with a maximum number of 4 characters. However, a character cannot
> be used that goes beyond the length of original vector, for example, `a.xyz` is an invalid swizzle for a variable of type
> `vec2` because it does not have a z component.
> Finally, vector swizzles will always return a vector or `float` type variable, the specific type is dependent on the swizzle, for example
> `a.xy` return a `vec2`, while `a.zyzy` returns a `vec4`. Appropriately, color swizzles only return color or `float` type variables.
> 
> ```
> float alpha = image("alpha_mask.png").a;  
> vec2 left_wall_uv = position().yz;
> color3 all_red = randomcolor().rrr;
> ```

### Port Access Operator

The port access operator either returns the value already assigned to a node port or assigns a value to that node port (only for input ports).
This is primarily useful for complex nodes such as `standard_surface` which have many inputs.

```
surfaceshader s = standard_surface();
s.base_color = color3{1, 0, 0};
s.specular_roughness = 0.2;
```

## Indexing Operator

TODO

## Precedence

| Order of Precedence (higher operations evaluate first)                  |
|-------------------------------------------------------------------------|
| `(a)`                                                                   |
| `a.b` `a[b]`                                                            |
| `a++` `a--` `++a` `--a`                                                 |
| `a += b` `a -= b` `a *= b` `a /= b` `a ^= b` `a %= b` `a &= b` `a \|= b` |
| `-a` `+a` `!a`                                                          |
| `a ^ b`                                                                 |
| `a * b` `a / b` `a % b`                                                 |
| `a + b` `a - b`                                                         |
| `a > b` `a >= b` `a < b` `a <= b`                                       |
| `a == b` `a != b`                                                       |
| `a & b` `a \| b`                                                        |

When two operators with equal precedence are used, the leftmost operator will evaluate first.  
As shown in the table above, precendence can be controlled using the grouping operator `(a)`. For example, in the expression
`a + b * c`, the `b * c` will evaluate first, however, in the expression `(a + b) * c`, the `a + b` will evaluate first.

The order of precedence of compound assignment operator (`a += b`, `a -= b`, etc.) only applies to the left hand side of 
the expression. The right hand side of the expression will always be evaluated first. For example:

```
int i = 0;
print 1 + 1 + i += 1 + 1;
// is equivalent to
print (1 + 1 + (i += (1 + 1)));
```

# Variable Definition

```
modifier-list type name = initial-value;
```   

`modifier-list` is a list of zero or more modifiers.  
`type` can be any primitive or user-defined data type.  
`name` can be any valid identifier.  
`initial-value` can be any valid expression that evaluates to `type`. It is optional.

### Examples

```
float a = 1.0;
float b = a;
float c;
bool half_chance = randomfloat() > 0.5;
int uv_channel = 3;
vec2 uv = 1.0 - texcoord(uv_channel);
string space = "world";
surfaceshader surface = standard_surface(

mutable int i = 0;
const float PI = 3.14;
global int iter_count = 50
```

## Modifiers

ShadingLanguageX supports three modifiers for variable declarations: `mutable`, `const` and `global`.

> [!TIP]
> ### New in mxslc++!
> ### Mutable
> 
> By default, variables in ShadingLanguageX are immutable (see [here](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/VariableMutability.md) for my reasoning behind this decision). To be able to change a variable after its initial definition, 
> use the `mutable` modifier.
> 
> ```
> int x = 1;
> x = 2; // Error
> 
> mutable int y = 1;
> y = 2; // OK
> ```

### Const

A variable declared with the `const` keyword cannot be assigned to after its initial definition. For example:

```
const int x = 1;
x = 2; // Error
```

This isn't particularly useful for primitive data types (except to improve readability of the code) as variables are immutable by default in ShadingLanguageX, but it has some use cases
with [user-defined types](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/mxslc++/LanguageSpecification.md#user-defined-types).

> [!WARNING]
> ### Not yet supported in mxslc++
> ### Global
> 
> The global keyword operates similarly to `uniform` from GLSL. Global variables do not require an initial value and are instead initialized by user-defined values passed to the compiler. For example:
> ```
> global filename albedo_path;
> global color3 tint;
> auto c = image<color3>(albedo_path) * tint;
> ```
> ```python
> from pathlib import Path
> import MaterialX as mx
> import mxslc
> globals = {
>     "albedo_path": Path(r"../brick.png"),
>     "tint": mx.Color3(1.0, 0.0, 0.0)
> }
> mxslc.compile_file("globals_example.mxsl", globals=globals)
> ```
> ```xml
> <?xml version="1.0"?>
> <materialx version="1.39">
>     <image name="node11" type="color3">
>         <input name="file" type="filename" value="..\brick.png" />
>     </image>
>     <multiply name="c" type="color3">
>         <input name="in1" type="color3" nodename="node11" />
>         <input name="in2" type="color3" value="1, 0, 0" />
>     </multiply>
> </materialx>
> ```

> [!TIP]
> ### New in mxslc++!
> ## Multi-Variable Definition
> 
> When returning a variable with a user-defined type from a function, the variable can either be bound to a single variable of the custom
> data type or split into its individual components. For example:
> 
> ```
> using Point = {float, float};
> 
> Point randompoint()
> {
>     return {randomfloat(), randomfloat()}; 
> }
> 
> 
> Point p = randompoint();
> 
> // OR
> float x, float y = randompoint();
> 
> // OR (only if x and y share the same type)
> float x, y = randompoint();
> ```

# Variable Assignment

```
name = value;
name[i] = value;
name.property = value;
```  
`name` must be the name of a previously declared variable.  
`value` can be any valid expression that evaluates to the type of `name`.  

### Example

```
mutable color3 albedo = image("albedo.png");

albedo = 1.0 - albedo;
```
```
using Point = {mutable float, mutable float};

Point p = {1.0, 2.0};

p[0] = 3.0; // OK
p[1] = 4.0; // OK

p[2] = 5.0; // Error: index out of range

int i = geompropvalue("i");
p[i] = 6.0; // Error: index not known at compile time

// Bonus
p = {7.0, 8.0}; // Error: p is not mutable, only its fields.
```
```
using Point = {float x, float y};

mutable Point p = {1.0, 2.0};

p.x = 3.0; // OK
p.y = 4.0; // OK

p.z = 5.0; // Error: invalid field name

// Bonus
p[0] = 6.0; // OK: indexing operator is still valid here, it will access the first field in the definition (x).
p = {7.0, 8.0}; // OK: both p and its fields can be assigned to because p is mutable.
```
```
surfaceshader s = standard_surface();
s.base_color = randomcolor();
s.specular_roughness = randomfloat();

// Equivalent to
surfaceshader s = standard_surface(base_color=randomcolor(), specular_roughness=randomfloat());

// Or
surfaceshader s = standard_surface(base_color=randomcolor());
s.specular_roughness = randomfloat();
```

> [!TIP]
> ### New in mxslc++!
> ## Increment/Decrement Operator
> 
> | Operation | Expands to                            |
> |-----------|---------------------------------------|
> | `i++;`    | Increase `i` by one after evaluation  |
> | `i--;`    | Decrease `i` by one after evaluation  |
> | `++i;`    | Increase `i` by one before evaluation |
> | `--i;`    | Decrease `i` by one before evaluation |
> 
> ### Example
> 
> ```
> int i = 0;
> print i++; // prints 0, then increases i by 1
> 
> int j = 0;
> print ++j; // increases j by 1, then prints 1
> ```

## Compound Assignment Operator

| Operation  | Expands to     |
|------------|----------------|
| `a += b;`  | `a = a + b;`   |
| `a -= b;`  | `a = a - b;`   |
| `a *= b;`  | `a = a * b;`   |
| `a /= b;`  | `a = a / b;`   |
| `a %= b;`  | `a = a % b;`   |
| `a ^= b;`  | `a = a ^ b;`   |
| `a &= b;`  | `a = a & b;`   |
| `a \|= b;` | `a = a \| b;`  |

### Note

Increment, Decrement and Compound Assignment Operators are expressions, not statements. This means you can use them anywhere you could normally use
an expression, such as an argument to a function. Compound Assignment Operators evaluate in the same order as prefix Increment/Decrement Operators,
i.e., they increase the value of the variable first, and then return the value.

```
int i = 0;
float a = randomfloat(seed = i+=10); // seed = 10
float b = randomfloat(seed = i+=10); // seed = 20
```

# Named Constructor

```
type{...}
```

`type` can be `bool` `int` `float` `vec2` `vec3` `vec4` `color3` `color4`.

The constructor has the same name as the type itself and accepts zero or more arguments.
The behaviour of the constructor changes depending on the number of arguments provided,
but will always return a variable of the corresponding type.

## Zero Arguments

In the case that no arguments are passed to the constructor, a default value will be returned.

| Data Type | Default Value                |
|-----------|------------------------------|
| `bool`    | `false`                      |
| `int`     | `0`                          |
| `float`   | `0.0`                        |
| `vec2`    | `vec2{0.0, 0.0}`             |
| `vec3`    | `vec3{0.0, 0.0, 0.0}`        |
| `vec4`    | `vec4{0.0, 0.0, 0.0, 1.0}`   |
| `color3`  | `color3{0.0, 0.0, 0.0}`      |
| `color4`  | `color4{0.0, 0.0, 0.0, 1.0}` |

## One Argument

With a single argument, the constructor will convert the argument to the target type.
Currently, this simply compiles to the `convert` node. For information regarding supported conversions,
see the MaterialX Standard Node document.

### Example

```
float is_shadow = float{x > y};
color3 white = color3{1.0};
color3 v_debug = color3{viewdirection()};
```

## Two Or More Arguments

For two or more arguments, the constructor builds the target type using the arguments in the order they are provided. 
If not enough arguments are provided, the remaining components will be zero.

### Example

`vec2 a = vec2{1.0, 2.0};`  
`vec3 b = vec3{a, 3.0}; // will be vec3{1.0, 2.0, 3.0}`  
`vec4 c = vec4{4.0, a}; // will be vec4{4.0, 1.0, 2.0, 0.0}`  
`color3 d = color3{g=1.0}; // will be color3{0.0, 1.0, 0.0}`

# If Expression

Unlike most languages, ShaderLanguageX has limited support for if statements but instead mainly uses if expressions. The reason for this is that conditional nodes (`ifequal`, `ifgreater` and `ifgreatereq`) in MaterialX act more like
ternary operators (`cond ? then : else`) than true if statements that control the logic of the program. 
This makes if expressions better suited for compiling to MaterialX nodes than if statements.

```
if (condition) { value_if_true } else { value_if_false }
```

`condition` can be any expression that evaluates to a bool type.  
The expression will evaluate to `value_if_true` if the condition is true, otherwise `value_if_false`.

The syntax `if (condition) { value_if_true }` can be used during an assignment statement. In this case, if the condition
evaluates to false, the variable will retain its original value.

### Examples

```
mutable float a = if (x > y) { 0.5 } else { 0.7 };

a = if (z > x) { 0.9 }; // keeps original value if z <= x
```  

```
vec3 upaxis = if (target_platform == UNREAL) 
{ 
    vec3{0.0, 0.0, 1.0} 
} 
else 
{ 
    vec3{0.0, 1.0, 0.0}
};
```

## Else If Expressions

Chained else if expressions are also possible. You can include any number of `else if` clauses in the expression.
```
float x = if (cond1) { val1 } else if (cond2) { val2 } else { val3 };
```
Once again you do not need to include the final `else` branch during a variable assignment. If none of the conditions evaluate
to true, then the variable will retain its previous value.
```
float x = 0.0;
x = if (cond1) { val1 } else if (cond2) { val2 };
```

### Example

```
vec3 upaxis = if (target_platform == UNREAL) 
{ 
    vec3{0.0, 0.0, 1.0} 
} 
else if (target_platform == ABSOLUTELY_NO_ONE)
{ 
    vec3{1.0, 0.0, 0.0}
}
else 
{ 
    vec3{0.0, 1.0, 0.0}
};
```

> [!TIP]
> ### New in mxslc++!
> # If Statement
> 
> ShadingLanguageX does support if statements; however, conditions must be evaluated at compile time, making them far less
> powerful than if statements in other languages.
> 
> ```
> if (condition)
> {
>     statement*
> }
> else if (condition)
> {
>     statement*
> }
> else
> {
>     statement*
> }
> ```
> 
> ### Example
> 
> ```
> if (true)
> {
>     // do something
> }
> else
> {
>     // do something else
> }
> ```
> 
> ```
> float a = geompropvalue("a");
> if (a > 0.5) // Error: runtime condition
> {
>     // do something
> }
> ```

> [!WARNING]
> ### Not yet supported in mxslc++
> # Switch Expression
> 
> ShaderLanguageX does not support switch statements, but instead uses switch expressions, for the same reasons as if
> expressions above. They are similar to switch expressions found in the C# programming language.
> 
> ```
> switch (which) { in1, in2, in3 }
> ```
> 
> `which` can be any expression that evaluates to either an `int` or `float` type.  
> The switch expression will evaluate to either `in1`, `in2`, `in3`, or a default value depending on the value of `which`.
> See the `switch` node in the MaterialX Standard Node document for more information.
> 
> ### Example
> 
> ```
> color3 albedo = switch (wall_id)
> {
>     image("left_wall.png", texcoord=uv),
>     image("right_wall.png", texcoord=uv),
>     image("back_wall.png", texcoord=uv),
>     image("ceiling.png", texcoord=uv),
>     image("floor.png", texcoord=uv)
> }
> ```

# Compile-Time Evaluation

Some expressions can be evaluated at compile-time if it's the case that all arguments are also known at compile-time. For example,
instead of `1 + 1` evaluating to an `add` node, it will simply evaluate to `2`. On the other hand, evaluating `geompropvalue<float>("x") + 1.0`
is not possible at compile-time because the user value is not known until render-time. The following list shows the MaterialX Nodes that
are currently optimised to evaluate at compile-time, if possible.

| Nodes       |
|-------------|
| `add`       |
| `subtract`  |
| `multiply`  |
| `divide`    |
| `combine2`  |
| `combine3`  |
| `combine4`  |
| `separate2` |
| `separate3` |
| `separate4` |
| `convert`   |

This also means that expressions or statements that require compile-time values such as if statements, for loops and 
indexing operators can use these expressions and not just literals. For example:

```
{float, float, float} x = {1.0, 2.0, 3.0};
print x[1+1]; // OK: 1+1 is evaluated at compile-time
```

# For Loop

Unlike if and switch expressions, loops are compiled as statements in ShadingLanguageX, with the caveat that the number of
loop iterations must be known at compile-time.

## For Range Loop

```
for (type name = start-value:end-value)
{
    statement*
}
```

`type` can either be `int` or `float`.  
`name` can be any valid identifier.  
`start-value` is the value that the iteration value will start from.  
`end-value` is the value that the iteration value will stop at.  
For example, `0:3` will iterate through the values `0` `1` `2` `3`.

For range loops can also be declared with an increment value. Instead of increasing the iteration value by
`1` each loop, it will be increased by the value of the specified increment instead. For example, `0:2:6` would result in the following sequence: `0` `2` `4` `6`.
Reverse iterators are also possible using this syntax, for example, `9:-1:0` would start at `9` and end at `0`.
```
for (type name = start-value:increment-value:end-value)
{
    statement*
}
```

### Example

```
// render 10 randomly sized white circles
mutable color3 c = color3{0.0};
for (int i = 0:9)
{
    vec2 center = vec2{randomfloat(in=0, seed=i), randomfloat(in=1, seed=i)};
    c = if (distance(center, texcoord()) < randomfloat(in=2, max=0.1, seed=i)) { color3{1.0} };
}
standard_surface(base_color=c);
```

## For Each Loop

For each loops iterate over a variables fields.

```
for (modifier-list type name = value)
{
    statement*
}
```

`modifier-list` is a list of zero or more modifiers.  
`type` can be any primitive or custom data type.  
`name` can be any valid identifier.  
`value` can be any valid expression.

### Example

```
{float, float, float} values = {1.0, 2.0, 3.0};

for (float v = values)
{
    print v;
}
```

# Functions

## Function Definition

Users can declare their own functions in ShadingLanguageX using the following syntax:
```
modifier-list type name(modifier-list param1_type param1_name, modifier-list param2_type param2_name, ...)
{
    statement*
}

OR

modifier-list function name(modifier-list param1_type param1_name, modifier-list param2_type param2_name, ...) -> type
{
    statement*
}
```

`modifier-list` is a list of zero or more modifiers.  
`type` can be any primitive or custom data type. It can also be `void` to indicate that function does not return a value.
In this case, the return statement in the body of the function should also be omitted.  
`name` can be any valid identifier.  
`paramN_type` can be any primitive or user-defined type and `paramN_name` can be any valid identifier. Functions can declare any number of parameters.   

## Calling Functions

`name(arg1, arg2, ...)`  
`name` is the name of function to be invoked.  
`argN` is N number of expressions whose data types match those in the function signature.

### Example

```
float mad(float m, float a, float b)
{
    return m * a + b;
}

float f = mad(1.0, 2.0, 3.0);
```
```
material main(color3 c, float intensity, vec3 n)
{
    return surfacematerial(
       standard_surface(
            base_color = c * intensity,
            normal = n
        )
    );
}

vec3 n = image("normals.png");
material m = main(color3(3.0, 7.0, 5.0), 0.8, n);
```
```
float i = 0.0;
void incr_i()
{
    i += 1.0;
}
incr_i(); // i == 1.0
incr_i(); // i == 2.0
```

## Function Scope

As well as the variables within them, functions can also access and modify variables from enclosing scopes,
as you saw in the previous example.

```
float a = 0.0;
void do_something()
{
    float b = 1.0;
    for (int i = 0:9)
    {
        float c = 2.0;
        a += b / c;
    }
}
```

In the above example, `a` is part of the global scope and is accessible everywhere, `b` is accessible only inside the `do_something`
function and the for loop, and `c` is accessible only within the for loop.

## Overloading Functions

Functions in ShadingLanguageX can be overloaded. This means that mutliple functions can have the same name, as long as their function signature is unique. When a function is called, the compiler will determine which of the functions to invoke based on the data types of the arguments passed to the function. For example:
```
float do_something(float f)
{
    return f * 2.0;
}

color3 do_something(color3 c)
{
    return c * 2.0;
}

color3 c = randomcolor();
c = do_something(c);
```
In this case, the call to `do_something()` will invoke the second function because it's being passed an argument of type `color3`.

It's important to note that in ShadingLanguageX, unlike other C-based languages, the return type of a function is included in its signature. For example:
```
vec2 texcoord()
{
    return position().xy;
}

vec3 texcoord()
{
    return position();
}

vec2 uv = texcoord();
```
In this case, the call to `texcoord()` will invoke the first function because it's return value is being assigned to a variable of type `vec2`.

## Default Parameter Values

Function parameters can be declared with a default value. In this case, the user is not required to provide a value for that parameter when calling the function.
```
float azimuth(string space = "world", vec3 origin = vec3{1.0, 0.0, 0.0})
{
    return dotproduct(normal(space), origin);
}

float theta = azimuth();
```

## Named Arguments

Function arguments can be named to target specific parameters.
```
float theta = azimuth(origin = vec3{0.0, 0.0, 1.0});
color3 albedo = image("albedo.png", texcoord = geompropvalue("uv4"));
```

## Templated Functions

It's common for functions to use the same logic on different data types. For example:
```
float one_minus(float v)
{
    return 1.0 - v;
}

vec2 one_minus(vec2 v)
{
    return 1.0 - v;
}
```

This type of code is time-consuming to both write and read. As such, ShadingLanguageX offers templated functions to streamline these types of functions.
```
T one_minus<float, vec2>(T v)
{
    return 1.0 - v;
}
```

This code is equivalent to the one above, but can also easily be extended to handle other data types as well.
```
T one_minus<float, vec2, vec3, vec4, color3, color4>(T v)
{
    return 1.0 - v;
}
```

In this case, the reserved keyword data type `T` is a placeholder for the data types specified in the template. `T` can be used anywhere in the function signature and inside the body of the function wherever a standard
data type could be used. For example:
```
T image_mult<vec3, color3>(filename img_path, T mult = T{1.0})
{
    T img = image<T>(img_path);
    return img * mult;
}
```

When calling a templated function, the template type can be inferred from the usage of the function, exactly as it works in function overloading. Additionally, the user can also explicitly specify the data type of the function template to use. For example:
```
T one_minus<float, vec2, vec3, vec4, color3, color4>(T v)
{
    return 1.0 - v;
}

vec2 uv = texcoord();

vec2 inv_uv = one_minus(uv); // <vec2> is inferred from return and parameter types
vec2 inv_uv = one_minus<vec2>(uv); // <vec2> can still be included for clarity
```

## Parameter Modifiers

Parameters (like variables) can be declared as `mutable` or `const`. They can also be declared as `out` or `ref`.

### Out Parameters

Including the `out` keyword before a parameter turns it into an out parameter. These parameters can then be assigned within the function, 
and the value will be accessible to the caller, similar to a return value. For example:

```
void sincos(const float r, out float s, out float c)
{
    s = sin(r);
    c = cos(r);
}

mutable float s = 0.0;
mutable float c = 0.0;

sincos(3.14, s, c);

// OR
sincos(3.14, out s, out c);
```

The `out` modifier can also be placed before the argument in the function call, but it's not required. If an argument is declared `out`, but the
corresponding parameter is not, the compiler will throw an error. You will also notice that I did not have to declare the out
parameters as `mutable` in the function definition, `out` parameters are mutable by default.

To compliment out parameters, variables can be declared as part of the function call (these are called variable declaration
expressions), meaning we don't need to separately declare the variables above the function. Variable declaration 
expressions do not need to be declared as `mutable`, unless you plan to modify them after the function call. For example, the above shader
can be rewritten as:

```
void sincos(const float r, out float s, out float c)
{
    s = sin(r);
    c = cos(r);
}

sincos(3.14, float s, float c);

// OR
sincos(3.14, out float s, out float c);
```

### Ref Parameters

`ref` parameters operate similarly to `out` parameters, but they also they retain the value of the variable passed into
the function, while `out` parameters always set the initial value to zero (or to the initial value of the parameter if provided).

```
void foo(out int x, ref int y)
{
    x *= 2;
    y *= 2;
}

mutable int x = 1;
mutable int y = 1;

foo(x, y);

// x == 0
// y == 2
```

## Inline Functions

Functions in ShadingLanguageX will normally compile to a `NodeDef`/`NodeGraph` pair. For example:
```
float add_one(float in)
{
    return in + 1.0;
}

float x = add_one(2.0);
float y = add_one(5.0);
```
Compiles to the following:
```xml
<nodedef name="ND_add_one" node="add_one">
    <output name="out" type="float" default="0.0" />
    <input name="in" type="float" value="0" />
</nodedef>
<nodegraph name="NG_add_one" nodedef="ND_add_one">
<add name="node3" type="float">
    <input name="in1" type="float" interfacename="in" />
    <input name="in2" type="float" value="1" />
</add>
<output name="out" type="float" nodename="node3" />
</nodegraph>
<add_one name="x" type="float">
<input name="in" type="float" value="2" />
</add_one>
<add_one name="y" type="float">
<input name="in" type="float" value="5" />
</add_one>
```
This can create a lot of overhead for small functions or functions you only plan to call once, or might be incompatible
with certain applications. The `inline` keyword instead forces the statements in a function to be created directly in the enclosing
scope. For example, inlining the function from earlier, like this:
```
inline float add_one(float in)
{
    return in + 1.0;
}

float x = add_one(2.0);
float y = add_one(5.0);
```
Instead, directly compiles to:
```xml
<add name="x" type="float">
    <input name="in1" type="float" value="2" />
    <input name="in2" type="float" value="1" />
</add>
<add name="y" type="float">
<input name="in1" type="float" value="5" />
<input name="in2" type="float" value="1" />
</add>
```

> [!TIP]
> ### New in mxslc++!
> ## Default Functions
> 
> Functions can be declared as `default` to break ties when a function call could match two potential function overrides.
> This was mainly implemented to make it more convenient to call some nodes from the MaterialX Standard Node library such as `randomfloat` and `randomcolor`
> which have overrides with very similar signatures.
> 
> ```
> vec3 foo(float in = 0.0)
> {
>     return vec3{in};
> }
> 
> vec3 foo(int in = 0)
> {
>     return vec3{float{in}};
> }
> 
> vec3 v = foo(); // Error: ambiguous function call
> ```
> 
> ```
> default vec3 foo(float in = 0.0)
> {
>     return vec3{in};
> }
> 
> vec3 foo(int in = 0)
> {
>     return vec3{float{in}};
> }
> 
> vec3 v = foo(); // OK: the default function is called
> ```

### Notes

* Functions can be declared inside other functions.
* Functions must be declared prior to being called.
* Recursion is not possible in ShadingLanguageX.
* MaterialX standard library nodes with multiple outputs (such as the separate nodes) return their values in an unnamed struct. For example:
```
// Signature: {float outx, float outy} separate2(const vec2 in)
float u, v = separate2(texcoord());
```

# User-Defined Types

## Unnamed Struct

The simplest user-defined type is the unnamed struct. It allows the user to declare a list of variables (known as fields) that are held inside a single container.
Field modifiers and types are declared the same way as a standard variable, but the name is optional. 

```
{modifier-list field_type1 field_name1?, modifier-list field_type2 field_name2?, ...}
```

`modifier-list` is a list of zero or more modifiers.  
`field_typeN` can be any primitive or user-defined type.  
`field_nameN` can be any valid identifier. It is optional.  
Unnamed structs can declare any number of fields.

Unnamed struct fields can be accessed using the indexing operator:

```
mutable {float, float} a = {1.0, 2.0};
print a[0];
print a[1];
a[0] = 3.0;
a[1] = 4.0;
```

If names are provided, they can be accessed using either the indexing and dot operator:

```
mutable {float x, float y} a = {1.0, 2.0};
print a[0];
print a.y;
a[0] = 3.0;
a.y = 4.0;
```

### Modifiers

Field modifiers work the same way as for variables:

```
{const float x, mutable float y} a = {1.0, 2.0};
a.x = 3.0; // Error: a.x is const
a.y = 4.0; // OK: x.y is mutable
a = {5.0, 6.0}; // Error: a is const (by default)
```

This is where `const` can be useful. If you have a field that you always want to be immutable, regardless of the status
of its container, you can declare it as `const` which overrides the mutability of its container, for example:

```
mutable {float x, const float y} a = {1.0, 2.0};
a.x = 3.0; // OK: x inherits its mutability from its container
a.y = 4.0; // Error: y is declared const, overriding `a`s mutability
```

The rule is that a field of a container will inherit the mutability of the container itself, unless overridden as part of that fields declaration.
This applies to unnamed structs, `using` statements and `class` definitions.

### Assignment and Function Passing

When assigning one unnamed struct to another, passing an unnamed struct as an argument to a function, or returning an
unnamed struct from a function, only the number of fields and field types are required to match. Field modifiers and names can be different
between the two unnamed structs. For example:

```
{float x, float y} a = {1.0, 2.0};
{float s, float t} b = a;

{float, float} foo({float u, float v} c)
{
    return {c.u * 2.0, c.v * 2.0};
}

{float i, float j} d = foo(b);

{vec3, string} e = d; // Error: types do not match
```

### Example

```
{vec2, float, color3} user_values = {
    geompropvalue("center"),
    geompropvalue("radius"),
    geompropvalue("color"),
};

color3 c = circle(center = user_values[0], radius = user_values[1]) * user_values[2];
```

```
vec3 get_end_of_ray({vec3 orig, vec3 dir} ray, float d)
{
    return ray.orig + ray.dir * d;
}

{vec3 orig, vec3 dir} ray = {vec3{0, 0, 0}, vec3{0, 0, 1}};

vec3 p = get_end_of_ray(ray, 10);
```

## Using Statement

Repeatedly typing out unnamed structs can be tedious and error-prone. `using` statements allow the user to create an alias
for an unnamed struct, which can be used in its place. For example:

```
using Ray = {vec3 orig, vec3 dir};

vec3 get_end_of_ray(Ray ray, float d)
{
    return ray.orig + ray.dir * d;
}

Ray ray = {vec3{0, 0, 0}, vec3{0, 0, 1}};

vec3 p = get_end_of_ray(ray, 10);
```

## Class Definition

Classes give the most customisation when creating custom data types. Users can define fields, methods and constructors 
within the class definition, with field and method definitions following the same rules as variable and function definitions.
The only difference being that fields do not need to be defined above a method for the method to be able to access them.
However, methods still need to be defined above another method in order to be called. Constructors follow a similar syntax
to C++ where the constructor name is the same as the class name.

```
class name
{
    // fields
    modifier-list field_type1 field_name1;
    
    // methods
    modifier-list type method_name(modifier-list param1_type param1_name, modifier-list param2_type param2_name, ...)
    {
        statement*
    }
    
    // constructors
    name(modifier-list param1_type param1_name, modifier-list param2_type param2_name, ...)
}
```

### Example

```
class Ray
{
    // constructor
    Ray(vec3 orig, vec3 dir)
    {
        this.orig = orig;
        this.dir = dir;
    }
    
    // method
    vec3 end_of_ray(float d)
    {
        return orig + dir * d;
    }
    
    // fields
    vec3 orig;
    vec3 dir;
}

Ray ray = Ray{vec3{0, 0, 0}, vec3{0, 0, 1}};
vec3 p = ray.end_of_ray(10);
```

## This

Class methods can access the class instance using the `this` keyword. They can also access the class instances fields directly. For example:

```
class Sphere
{
    vec2 center;
    float radius;
    
    float area()
    {
        return 4.0 * PI * radius * radius;
        // OR
        return 4.0 * PI * this.radius * this.radius;
    }
    
    Sphere grow(float amount)
    {
        // copy instance
        Sphere result = this;
        
        result.radius += amount;
        return result;
    }
}
```

# Unnamed Constructor

As you've seen throughout this document, user-defined types can be initialised using the unnamed constructor, compared to primitive types
which are initialised using the named constructor. Class types can be initialised using either.

A named constructor is a specific, defined function, either one that has been defined as part of a class definition,
or, in the case of primitive types, is defined in the ShadingLanguageX Standard Library. While an unnamed constructor simply
evaluates the arguments passed to it, assuming the number of arguments and their types match the target type.

```
{float, float} x = {1.0, 2.0}; // OK: unnamed constructor evaluates to {float, float}, matching the target type

class Point
{
    float x;
    float y;
}

Point p = {3.0, 4.0}; // OK: unnamed constructor matches Points field structure

Point q = {"hello", "world"}; // Error: unnamed constructor does not match Points field structure
```

# Operator Overloading

Operators can be overloaded for all user-defined types, including unnamed structs, aliases and class types.
Operator overload functions are defined outside of class definitions.

| Operation | Function Name       |
|-----------|---------------------|
| `a + b`   | `__add__`           |
| `a - b`   | `__sub__`           |
| `a * b`   | `__mul__`           |
| `a / b`   | `__div__`           |
| `a % b`   | `__mod__`           |
| `a ^ b`   | `__pow__`           |
| `a > b`   | `__gt__`            |
| `a >= b`  | `__ge__`            |
| `a < b`   | `__lt__`            |
| `a <= b`  | `__le__`            |
| `a == b`  | `__eq__`            |
| `a != b`  | `__ne__`            |
| `a & b`   | `__and__`           |
| `a \| b`  | `__or__`            |
| `!a`      | `__not__`           |
| `-a`      | `__neg__`           |
| `+a`      | `__pos__`            |

```
using Point = {float x, float y};


Point p = {1.0, 2.0};
Point q = {3.0, 4.0};
Point r = p + q; // Error: __add__ not defined for Points


Point __add__(Point a, Point b)
{
    return {a.x + b.x, a.y + b.y};
}

Point r = p + q; // OK


Point __add__(Point a, float b)
{
    return {a.x + b, a.y + b};
}

r += 10.0; // OK
``` 

> [!TIP]
> ### New in mxslc++!
> # Print Statement
> 
> Print statements are used to output data to the console. They can be used to debug expression values during the compilation
> of the shader.
> 
> ```
> print "hello world";
>     > hello world
>     
> print 1 + 1;
>     > 2
>     
> print randomcolor();
>     > <randomcolor name="node1" type="color3" />
>     
> float u, v = separate2(texcoord());
> print u;
>     > <output name="outx" type="float" />
> ```
> 
> Multiple expressions can be passed, with each printing on a new line:
> 
> ```
> print 1 + 1, 2 + 2, 3 + 3;
>     > 2
>     > 4
>     > 6
> ```

# Attributes

Attributes can be defined above statements or parameters and are added to the element that the statement compiles into. For example,
attributes defined above a function declaration will be added to the compiled NodeDef.
```
@nodegroup "math"
@doc "adds one to in"
float add_one(float in)
{
    return in + 1.0;
}
```
```xml
<nodedef name="ND_add_one" node="add_one" nodegroup="math" doc="adds one to in">
    <output name="out" type="float" default="0.0" />
    <input name="in" type="float" value="0" />
</nodedef>
<nodegraph name="NG_add_one" nodedef="ND_add_one">
<add name="node3" type="float">
    <input name="in1" type="float" interfacename="in" />
    <input name="in2" type="float" value="1" />
</add>
<output name="out" type="float" nodename="node3" />
</nodegraph>
```
Attributes can also be added to the input/output elements of a NodeDef by specifying the name of the port before the attribute
name. Return values are called `out` to match the default in MaterialX.
```
@nodegroup "math"
@doc "adds one to in"
@in.doc "the value to be incremented"
@out.doc "the incremented value"
float add_one(float in)
{
    return in + 1.0;
}
```
```xml
<nodedef name="ND_add_one" node="add_one" nodegroup="math" doc="adds one to in">
    <output name="out" type="float" default="0.0" doc="the incremented value" />
    <input name="in" type="float" value="0" doc="the value to be incremented" />
</nodedef>
<nodegraph name="NG_add_one" nodedef="ND_add_one">
<add name="node3" type="float">
    <input name="in1" type="float" interfacename="in" />
    <input name="in2" type="float" value="1" />
</add>
<output name="out" type="float" nodename="node3" />
</nodegraph>
```
An alternative syntax is to place the attributes directly above parameters. This can be useful for functions with a large
number of parameters to keep attribute close to their associated parameter.
```
@nodegroup "math"
@doc "adds one to in"
@out.doc "the incremented value"
float add_one(
    @doc "the value to be incremented"
    float in
)
{
    return in + 1.0;
}
```
Finally, attributes can also be defined above variable declarations, variable assignments and expression statements. These
attributes will be added to the nodes that these statements compile into.
```
@doc "an image of a butterfly"
@file.colorspace "srgb_texture"
color3 c = image("butterfly1.png");

// OR

@doc "an image of a butterfly"
color3 c = image(
    @colorspace "srgb_texture"
    "butterfly1.png"
);
```
```xml
<image name="c" type="color3" doc="an image of a butterfly">
    <input name="file" type="filename" colorspace="srgb_texture" value="butterfly1.png" />
</image>
```

> [!TIP]
> ### New in mxslc++!
> ## Document Attributes
> 
> To apply attributes at the document level, use `@@` instead of `@`.
> 
> ```
> @@madewith "shadinglanguagex"
> 
> standard_surface(base_color = randomcolor());
> ```
> 
> ```xml
> <?xml version="1.0"?>
> <materialx version="1.39" madewith="shadinglanguagex">
>   <randomcolor name="node1" type="color3" />
>   <standard_surface name="node2" type="surfaceshader">
>     <input name="base_color" type="color3" nodename="node1" />
>   </standard_surface>
> </materialx>
> 
> ```

> [!WARNING]
> ### Not yet supported in mxslc++
> # Node Constructor
> 
> Node constructors are a unique expression to ShadingLanguageX.
> 
> ```
> {string, type: input1=value1, input2=value2...}
> ```  
> `string` can be any valid string value.    
> `type` can be any supported data type.  
> `inputN` can be any valid identifier.
> `valueN` can be any valid expression.
> 
> Node constructors compile to the node element specified by `string` and the type specified by `type`. Node inputs are compiled from
> the list of inputs that come after the colon `:`.
> 
> Node constructors give developers the ability to define any node that they want, regardless of whether it is implemented
> in ShadingLanguageX or not. For example, the `normalmap` node from the MaterialX Standard Node specification changed signature
> in v1.39. However, many renderers are still using the v1.38 signature. Node constructors can be used to create node elements
> with an old input signature to ensure compatability with as many renderers as possible, or to declare nodes that are not defined
> in the MaterialX specification, such as renderer specific nodes.
> 
> ### Example
> 
> #### Normalmap compatability
> ```
> vec3 normals = image("normals.png");
> 
> // v1.38 normalmap
> vec3 nm = {"normalmap", vec3: in=normals, space="tangent", scale=0.1};
> ```
> compiles to:
> ```
> <normalmap name="nm" type="vector3">
>   <input name="in" type="vector3" nodename="normals" />
>   <input name="space" type="string" value="tangent" />
>   <input name="scale" type="float" value="0.1" />
> </normalmap>
> ...
> ```
> 
> #### Renderer-specific nodes
> ```
> // Houdini bias node
> float bias = {"hmtlxbias", float: in=0.0, bias=0.5};
> ```
> 
> ### Note
> 
> Unlike the rest of ShadingLanguageX, node constructors do not perform any type checking. In fact, the data type of the inputs
> is determined by the values that are passed to them.

# Null Expression

MaterialX defines default values for all node inputs in the standard library. To use these defaults instead of 
having to provide your own, you can use the `null` keyword. Essentially the `null` value will either not create or even delete the input port
from the node, so the MaterialX default is used instead.

```
surfaceshader s = standard_surface(base = 0.8, base_color = color3{1, 0, 0});

print s;
    > <standard_surface name="node2" type="surfaceshader">
    >   <input name="base" type="float" value="0.8" />
    >   <input name="base_color" type="color3" value="1, 0, 0" />
    > </standard_surface>
  
s.base_color = null;

print s;
    > <standard_surface name="node2" type="surfaceshader">
    >   <input name="base" type="float" value="0.8" />
    > </standard_surface>
  
```

Null expressions are also useful as default parameter values where it makes sense to use the default MaterialX behaviour.
```
{float, float} randompoint(float in=null, float min=null, float max=null)
{
    return {
        randomfloat(in=in, min=min, max=max, seed=1),
        randomfloat(in=in, min=min, max=max, seed=2),
    };
}
```

# Standard Library

ShadingLanguageX provides access to the nodes specified in the [MaterialX Data Library](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/libraries) as a standard library of functions.
More information about these nodes can be found in the MaterialX StandardNodes, PBRSpec and NPRSpec [documents](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/documents/Specification).
They are included automatically by the compiler and don't need to be manually included with the `#include` directive. For example:

```
vec3 i = viewdirection();
color3 c = randomcolor();
```

Function templates are added based on the name of the NodeDef, for example, the `texcoord` node has two NodeDefs:
`ND_texcoord_vector2` and `ND_texcoord_vector3`, which provide the two functions:

```
auto uv = texcoord<vec2>();
auto uvw = texcoord<vec3>();
```

Finally, any standard library functions that return multiple values, like `separate2`, return all values in an unnamed struct:

```
vec2 uv = texcoord();
float u, v = separate2(uv);
```

# Scope

ShadingLanguageX follows the same scoping rules as most C-based languages. Scopes are denoted by a pair of curly braces `{` `}` like in function declarations or for loops.
Variables can be accessed and modified inside their own scope as well as any nested scopes.
```
float a = 0.0;
void do_something()
{
    float b = 1.0;
    for (int i = 0:9)
    {
        float c = 2.0;
        a += b / c;
    }
}
```
In the previous example, `a` is part of the global scope and is accessible everywhere, `b` is accessible only inside the `do_something`
function and the for loop, and `c` is accessible only within the for loop.  
It's also possible to hide variables or functions from outer scopes by defining them again inside a nested scope.
```
float a = 1.0;
float b = 2.0;
float foo(float a = 3.0)
{
    float b = 4.0;
    print a, b;
        > 3.0 // nonlocal hidden by parameter a
        > 4.0 // nonlocal hidden by local variable b
}
```

```
void foo()
{
    print "foo";
}

void bar()
{
    void foo()
    {
        print "cooler foo";
    }
    
    foo();
        > "cooler foo";
}
```

Variable and function hiding can be used to override the functionality of standard library functions as they are defined in a 
scope above the global document scope.

> [!WARNING]
> ### Not yet supported in mxslc++
> # Preprocessor Directives
> 
> ShadingLanguageX supports many of the C preprocessor directives:
> * File inclusion (`#include`)
> * Macro definition (`#define` `#undef`)
> * Conditional compilation (`#if` `#ifdef` `#ifndef` `#elif` `#else` `#endif`)
> 
> ## File Inclusion
> 
> The `#include` directive allows users to include other `.mxsl` or `.mtlx` files in the current compilation, giving them access to
> any Nodes or NodeDefs defined in those files. The directive also supports the inclusion of directories as well as specific files. All files
> with the extension `.mxsl` or `.mtlx` inside the directory will be included. The search is not recursive, i.e., it does not include
> files from subdirectories. Files are not included in any particular order; if the order of included files is important,
> then the user should include each file individually in the necessary order.
> 
> When including a file, paths may be absolute or relative. If the path is relative, ShadingLanguageX will look for the file in
> the following directories in order:
> 1. Any additional directories passed to the compiler (see Command Line Options below)
> 2. The parent directory of the `.mxsl` file currently being compiled.
> 3. The parent directory of the mxslc executable.
> 
> ### Example
> 
> _color_enums.mxsl_:
> ```
> const color3 RED = color3(1.0, 0.0, 0.0)
> const color3 GREEN = color3(0.0, 1.0, 0.0)
> const color3 BLUE = color3(0.0, 0.0, 1.0)
> ```
> 
> _file_incl_example.mxsl_:
> ```
> #include "color_enums.mxsl"
> vec2 uv = texcoord();
> color3 c = mix(mix(RED, GREEN, uv.x), BLUE, uv.y);
> standard_surface(base_color=c);
> ```
> 
> ## Loadlib
> 
> The `#loadlib` directive looks for NodeDef elements in specified `.mtlx` or `.mxsl` files and allows users to call them
> from the source file being compiled. The NodeDefs are __not__ included in the compiled file, similar
> to how the MaterialX Standard Nodes are not included in compiled SLX files.
> 
> The `#loadlib` directive follows the same rules as the `#include` directive when searching for files (see above).
> 
> The directive takes an optional list of function names after the specified path. When included, the compiler will only
> load functions with the specified names.
> 
> ```
> // loads all functions from colors.mxsl
> #loadlib "colors.mxsl"
> 
> // loads only mad and pi from math.mtlx
> #loadlib "math.mtlx" (mad, pi)
> ```
> 
> ## Macro Definition
> 
> Macros are more limited in ShadingLanguageX than in C. Only flag- and object-like macros are supported, e.g.,
> `#define USE_FFT` and `#define PI 3.14159`. Function-like macros as well as macro stringification and token pasting are not supported.
> 
> There is currently one macro pre-defined during shader compilation:
> * `__MAIN__` is defined during compilation of the file originally given to the compiler.
> 
> `> ./mxslc.exe macro_def_example.mxsl`
> ```
> // macro_def_example.mxsl
> #ifdef __MAIN__ // will be defined in this context
> #include "macro_def_incl.mxsl"
> #endif
> ```
> ```
> // macro_def_incl.mxsl
> #ifdef __MAIN__ // will not be defined in this context
> ...
> #endif
> ```
> 
> ## Conditional Compilation
> 
> Conditional compilations directives operate as they do in C with no notable changes.
> 
> ## Version
> 
> The `#version` directive can be used to tell the compiler which version of MaterialX it should target when loading the standard
> library and validating the compiled .mtlx file. It also sets the version string of the compiled .mtlx file. You can
> specify the full version number, e.g., `#version 1.38.10`, or simply specify the major and minor version numbers,
> e.g., `#version 1.38`, in which case the most recent patch number will be added.
> 
> ### Notes
> 
> * You can omit the `#version` directive, in which case the compiler will use the most recent version of MaterialX (currently 1.39.3).
> * The `#version` directive is ignored if the file is being imported using the `#include` directive.
> * The version can also be specified as a compiler option. The compiler option overrides the `#version` directive if the two versions do not match.
> 
> ### Example
> 
> ```
> #version 1.38
> 
> // deprecated swizzle node
> vec2 vu = swizzle(texcoord<vec2>(), "yx");
> ```
