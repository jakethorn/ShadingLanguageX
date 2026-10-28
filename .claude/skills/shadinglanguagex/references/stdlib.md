# Standard library: MaterialX nodes as SLX functions

Every NodeDef in the target MaterialX data library (stdlib, pbrlib, bxdf, nprlib, lights, cmlib) is
callable without any `#include`. The listing at the end was generated from the MaterialX **1.39.5**
NodeDefs using the compiler's own mapping rules. Every line was compile-checked with both positional and
named arguments.

## How NodeDefs become functions

- **Name**: the node category, as in `<image>` becoming `image(...)`.
- **Parameters**: the NodeDef inputs, in order, with the same names. Every parameter defaults to `null`,
  so the input is left unset and MaterialX's default applies. Pass arguments positionally, by name
  (`image("a.png", texcoord = uv)`), or mixed with positional ones first.
- **Return value**: the output type. Nodes with several outputs return an unnamed struct whose fields are
  the outputs: `float x, y = separate2(v);` or `separate2(v).outx`.
- **Overloads**: each type variant (`ND_image_color3`, `ND_image_float`, ...) is an overload. Resolution
  uses the argument types **and the expected result type**, so give the call a typed destination or an
  explicit template.
- **Explicit template** `name<T>(...)`: available when the NodeDef name ends in a type
  (`ND_texcoord_vector2` gives `texcoord<vec2>()`, and `ND_extract_vector3` gives `extract<vec3>(v, 1)`).
  NodeDefs with other suffixes (`ND_add_color3FA`, `ND_noise2d_color3FA`, `ND_randomfloat_integer`)
  are reached through ordinary overload resolution, or through their own suffix type where one exists,
  as with `randomfloat<int>(3)`.
- **Tie-breaks**: when several NodeDefs share a category and return type, the one whose name sorts first
  is marked `default`. That is how `randomfloat()` (float `in` rather than int) and `noise2d<color3>()`
  (vec3 amplitude rather than float) resolve.
- `#version` changes the node set. For example, the deprecated `swizzle` node only exists in 1.38.
- **Uncallable**: the compositing node `out(fg, bg, mix)` can't be called because `out` is an SLX
  keyword. `switch(...)` works as a call as long as it isn't followed by `{`.

Listing notation:

- `T` or `V` is a type variable. The values it takes are listed after the signature.
- `<UV0>`, `<Pobject>`, `<Nworld>` and similar mean the input defaults to that geometric property, so
  `texcoord` defaults to UV set 0.
- `{a|b|c}` lists the allowed enum values of a string or int input.
- `= x` is the MaterialX default, shown as one component when all components are equal (`color3 c = 0.8`
  means `(0.8, 0.8, 0.8)`).

## SLX-only extensions (from `libraries/stdlib.mxsl`)

| Signature | Notes |
|---|---|
| `T min(float in1, T in2)`, `T max(float in1, T in2)` | float-first forms, for T in vec2-4 and color3/4 |
| `T min(T a, T b, T c[, T d[, T e]])`, `max(...)` | 3 to 5 arguments, for T in float, vec2-4 and color3/4 |
| `separate(v)` | returns separate2/3/4's struct for any vector or colour. **Bug:** the vec3 and vec4 versions name their fields `{outx, outz, outz}` / `{outx, outz, outz, outw}`, so destructure (`float x, y, z = separate(v);`) or use `separate3` |
| `combine(x, y[, z[, w]])` | builds vec2/vec3/vec4/color3/color4 depending on the destination type |
| type aliases | `bool int vec2 vec3 vec4 mat3 mat4` |
| operators | `__add__`, `__sub__`, ... wrap `add`, `subtract`, ... (see language.md §6) |
| named constructors | `__vector3__` and similar, which implement `vec3{...}` |
| component access | `__get__` and `__set__`, which implement `v[i]` |

## GLSL / HLSL / OSL to SLX

| Source | SLX |
|---|---|
| `vec3(a, b, c)`, `float(i)`, `vec3(1.0)` | `vec3{a, b, c}`, `float{i}`, `vec3{1.0}` |
| `mat3(a, b, c)` | `creatematrix(a, b, c)`. Each `vec3` is one GLSL column (in genglsl). Identity is `default(mat3)` |
| `c ? a : b` | `if (c) { a } else { b }` |
| `&&`, `\|\|` | `&`, `\|` (same precedence, so parenthesise) |
| `abs(x)` | `absval(x)` or `\|x\|` |
| `pow(x, y)` | `power(x, y)` or `x ^ y` |
| `mod(x, y)`, `fmod` | `modulo(x, y)` or `x % y` (float and vector only) |
| `fract`, `frac` | `fract` |
| `log(x)` | `ln(x)` |
| `atan(y, x)`, `atan2` | `atan2(y, x)` (inputs `iny`, `inx`) |
| `inversesqrt(x)` | `1.0 / sqrt(x)` |
| `length(v)` | `magnitude(v)` |
| `dot(a, b)` | `dotproduct(a, b)`. `dot()` is a pass-through node |
| `cross(a, b)` | `crossproduct(a, b)` |
| `mix(a, b, t)`, `lerp(a, b, t)` | `mix(b, a, t)`. MaterialX order is `mix(fg, bg, mix)` = `fg*mix + bg*(1-mix)` |
| `saturate(x)` (HLSL) | `clamp(x, 0.0, 1.0)`. MaterialX `saturate` adjusts colour saturation |
| `step(edge, x)` | `float{x >= edge}` |
| `smoothstep(e0, e1, x)` | `smoothstep(x, e0, e1)` (MaterialX order is `(in, low, high)`), **only when `e0 < e1`**. MaterialX returns 1 whenever `x >= high`, so reversed edges, common in Shadertoy code, need a helper: `inline float glsl_smoothstep(float e0, float e1, float x) { float t = clamp((x - e0) / (e1 - e0), 0.0, 1.0); return t * t * (3.0 - 2.0 * t); }` |
| `texture(tex, uv)` | `image(file, texcoord = uv)`, or `tiledimage(...)` |
| `normalize`, `reflect(I, N)`, `refract(I, N, eta)`, `distance`, `clamp`, `min`, `max`, `floor`, `ceil`, `round`, `sign`, `sqrt`, `exp`, `sin` ... `acos` | same names (`refract(in, normal, ior)`) |
| `m * v` | `transformmatrix(v, m)` (inputs `in`, `mat`) |
| `uniform float x;` | `global float x;` (compile-time) or `geomprop float x;` (per-geometry, runtime) |
| `in`/`out`/`inout` parameters | `out` / `ref` |
| arrays | tuples or structs (`{float, float, float} a = {...}; a[i]`) |
| `for (int i = 0; i < N; i++)` | `for (int i from 0 to N - 1)` |
| `fragCoord`, `iResolution` | `texcoord()` (0-1) or `position()`; `vec2{1, 1}` |
| `iTime` | `time()` |
| `discard`, `while`, `break`, recursion, derivatives | not available |

## Most-used nodes

- **Geometry**: `position(space)`, `normal(space)`, `tangent()`, `bitangent()`, `texcoord<vec2>(index)`,
  `geomcolor()`, `geompropvalue<T>("name", default)`, `viewdirection(space)` (nprlib), `time()`,
  `frame()`.
- **Textures**: `image<T>(file, layer, default, texcoord, ...)`, `tiledimage<T>(...)`,
  `hextiledimage<T>(...)`, `triplanarprojection<T>(...)`, `latlongimage(...)`.
- **Noise**: `noise2d<T>(amplitude, pivot, texcoord)`, `noise3d<T>(amplitude, pivot, position)`,
  `fractal2d/3d<T>(amplitude, octaves, lacunarity, diminish, ...)`, `cellnoise2d/3d`,
  `worleynoise2d/3d<T>`, `unifiednoise2d/3d`, `randomfloat`, `randomcolor`.
- **Patterns**: `checkerboard`, `circle`, `line`, `hexagon`, `grid`, `crosshatch`, `tiledcircles`,
  `ramp4`, `ramplr`, `ramptb`, `splitlr`, `splittb`, `place2d`.
- **Shaders**: `standard_surface`, `open_pbr_surface`, `gltf_pbr`, `UsdPreviewSurface`,
  `disney_principled`, `surface_unlit`, then `surfacematerial(surface, backsurface, displacement)` and
  `displacement(height_or_vector, scale)`. BSDF building blocks such as `oren_nayar_diffuse_bsdf`,
  `dielectric_bsdf`, `conductor_bsdf`, `layer` and `mix` feed `surface(bsdf, edf, opacity, thin_walled)`.
- **Colour**: `luminance`, `hsvadjust`, `rgbtohsv`, `hsvtorgb`, `contrast`, `colorcorrect`,
  `saturate`, `remap`, `range`, `srgb_texture_to_lin_rec709` (cmlib).

---

# Full listing (MaterialX 1.39.5)

## stdlib

- `T absval<T>(T in = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T acos<T>(T in = 0.0)` — T: float|vec2|vec3|vec4
- `T add(T in1, T in2)` — T: BSDF|EDF|VDF
- `T add<T>(T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T add(T in1, float in2 = 0.0)` — T: vec2|vec3|vec4|color3|color4|mat3|mat4
- `bool and(bool in1 = false, bool in2 = false)`
- `T asin<T>(T in = 0.0)` — T: float|vec2|vec3|vec4
- `T atan2<T>(T iny = 0.0, T inx = 1.0)` — T: float|vec2|vec3|vec4
- `vec3 bitangent(string space = "object" {model|object|world}, int index = 0)`
- `T blur<T>(T in = 0.0, float size = 0.0, string filtertype = "box" {box|gaussian})` — T: float|vec2|vec3|vec4|color3|color4
- `vec3 bump(float height = 0, float scale = 1, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, vec3 bitangent = <Bworld>)`
- `T burn<T>(T fg = 0.0, T bg = 0.0, float mix = 1.0)` — T: float|color3|color4
- `T ceil<T>(T in = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `int ceil<int>(float in = 0.0)`
- `float cellnoise2d(vec2 texcoord = <UV0>)`
- `float cellnoise3d(vec3 position = <Pobject>)`
- `color3 checkerboard(color3 color1 = 1.0, color3 color2 = 0.0, vec2 uvtiling = 8, vec2 uvoffset = 0, vec2 texcoord = <UV0>)`
- `float circle(vec2 texcoord = <UV0>, vec2 center = 0, float radius = 0.5)`
- `T clamp<T>(T in = 0.0, T low = 0.0, T high = 1.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T clamp(T in = 0.0, float low = 0.0, float high = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `float cloverleaf(vec2 texcoord = <UV0>, vec2 center = 0, float radius = 0.5)`
- `T colorcorrect<T>(T in, float hue = 0, float saturation = 1, float gamma = 1, float lift = 0, float gain = 1, float contrast = 1, float contrastpivot = 0.5, float exposure = 0)` — T: color3|color4
- `vec2 combine2<vec2>(float in1 = 0.0, float in2 = 0.0)`
- `color4 combine2(color3 in1 = 0.0, float in2 = 0.0)`
- `vec4 combine2(vec3 in1 = 0.0, float in2 = 0.0)`
- `vec4 combine2(vec2 in1 = 0.0, vec2 in2 = 0.0)`
- `T combine3<T>(float in1 = 0.0, float in2 = 0.0, float in3 = 0.0)` — T: vec3|color3
- `T combine4<T>(float in1 = 0.0, float in2 = 0.0, float in3 = 0.0, float in4 = 0.0)` — T: vec4|color4
- `T constant<T>(T value)` — T: bool|int|float|vec2|vec3|vec4|color3|color4|mat3|mat4|string|filename
- `T contrast<T>(T in = 0.0, T amount = 1.0, T pivot = 0.5)` — T: float|vec2|vec3|vec4|color3|color4
- `T contrast(T in = 0.0, float amount = 1.0, float pivot = 0.5)` — T: vec2|vec3|vec4|color3|color4
- `T convert<T>(float in)` — T: vec2|vec3|vec4|color3|color4|surfaceshader
- `T convert<T>(color3 in)` — T: vec2|vec3|vec4|color4|surfaceshader
- `T convert<T>(color4 in)` — T: vec2|vec3|vec4|color3|surfaceshader
- `T convert<T>(vec2 in)` — T: vec3|vec4|color3|color4|surfaceshader
- `T convert<T>(vec3 in)` — T: vec2|vec4|color3|color4|surfaceshader
- `T convert<T>(vec4 in)` — T: vec2|vec3|color3|color4|surfaceshader
- `T convert<T>(bool in = false)` — T: int|float|vec2|vec3|vec4|color3|color4|surfaceshader
- `T convert<T>(int in = 0)` — T: bool|float|vec2|vec3|vec4|color3|color4|surfaceshader
- `T cos<T>(T in = 0.0)` — T: float|vec2|vec3|vec4
- `mat3 creatematrix<mat3>(vec3 in1 = (1.0, 0.0, 0.0), vec3 in2 = (0.0, 1.0, 0.0), vec3 in3 = (0.0, 0.0, 1.0))`
- `mat4 creatematrix(V in1, V in2, V in3, V in4)` — V: vec3|vec4
- `color3 crosshatch(vec2 texcoord = <UV0>, vec2 uvtiling = 1.0, vec2 uvoffset = 0.0, float thickness = 0.05, bool staggered = false)`
- `vec3 crossproduct(vec3 in1 = 0.0, vec3 in2 = 0.0)`
- `float determinant<V>(V in)` — V: mat3|mat4
- `T difference<T>(T fg = 0.0, T bg = 0.0, float mix = 1.0)` — T: float|color3|color4
- `color4 disjointover(color4 fg = 0.0, color4 bg = 0.0, float mix = 1.0)`
- `float distance<V>(V in1 = 0.0, V in2 = 0.0)` — V: vec2|vec3|vec4
- `T divide<T>(T in1, T in2)` — T: float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T divide(T in1 = 0.0, float in2 = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `T dodge<T>(T fg = 0.0, T bg = 0.0, float mix = 1.0)` — T: float|color3|color4
- `T dot<T>(T in, string note = "")` — T: bool|int|float|vec2|vec3|vec4|color3|color4|mat3|mat4|string|filename|displacementshader|lightshader|surfaceshader|volumeshader
- `float dotproduct<V>(V in1 = 0.0, V in2 = 0.0)` — V: vec2|vec3|vec4
- `T exp<T>(T in = 0.0)` — T: float|vec2|vec3|vec4
- `float extract<color3>(color3 in = 0.0, int index = 0)`
- `float extract<color4>(color4 in = 0.0, int index = 0)`
- `float extract<vec2>(vec2 in = 0.0, int index = 0)`
- `float extract<vec3>(vec3 in = 0.0, int index = 0)`
- `float extract<vec4>(vec4 in = 0.0, int index = 0)`
- `vec3 extract<mat3>(mat3 in = 0.0, int index = 0)`
- `vec4 extract<mat4>(mat4 in = 0.0, int index = 0)`
- `{int id, float rand, float presence, vec3 flakenormal} flake2d(float size = 0.01, float roughness = 0.1, float coverage = 0.5, vec2 texcoord = <UV0>, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, vec3 bitangent = <Bworld>)`
- `{int id, float rand, float presence, vec3 flakenormal} flake3d(float size = 0.01, float roughness = 0.1, float coverage = 0.5, vec3 position = <Pobject>, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, vec3 bitangent = <Bworld>)`
- `T floor<T>(T in = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `int floor<int>(float in = 0.0)`
- `T fract<T>(T in = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T fractal2d<T>(T amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec2 texcoord = <UV0>)` — T: float|vec2|vec3|vec4
- `color3 fractal2d<color3>(vec3 amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec2 texcoord = <UV0>)`
- `color4 fractal2d<color4>(vec4 amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec2 texcoord = <UV0>)`
- `T fractal2d(float amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec2 texcoord = <UV0>)` — T: vec2|vec3|vec4|color3|color4
- `T fractal3d<T>(T amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec3 position = <Pobject>)` — T: float|vec2|vec3|vec4
- `color3 fractal3d<color3>(vec3 amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec3 position = <Pobject>)`
- `color4 fractal3d<color4>(vec4 amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec3 position = <Pobject>)`
- `T fractal3d(float amplitude = 1.0, int octaves = 3, float lacunarity = 2.0, float diminish = 0.5, vec3 position = <Pobject>)` — T: vec2|vec3|vec4|color3|color4
- `float frame()`
- `T geomcolor<T>(int index = 0)` — T: float|color3|color4
- `T geompropvalue<T>(string geomprop = "", T default)` — T: bool|int|float|vec2|vec3|vec4|color3|color4
- `T geompropvalueuniform<T>(string geomprop = "", T default = "")` — T: string|filename
- `color3 grid(vec2 texcoord = <UV0>, vec2 uvtiling = 1.0, vec2 uvoffset = 0.0, float thickness = 0.05, bool staggered = false)`
- `vec3 heighttonormal(float in = 0.0, float scale = 1.0, vec2 texcoord = <UV0>)`
- `float hexagon(vec2 texcoord = <UV0>, vec2 center = 0, float radius = 0.5)`
- `T hextiledimage<T>(filename file = "", T default = 0.0, vec2 texcoord = <UV0>, vec2 tiling = 1.0, float rotation = 1.0, vec2 rotationrange = (0.0, 360.0), float scale = 1.0, vec2 scalerange = (0.5, 2.0), float offset = 1.0, vec2 offsetrange = (0.0, 1.0), float falloff = 0.5, float falloffcontrast = 0.5, color3 lumacoeffs = (0.2722287, 0.6740818, 0.0536895) {acescg|rec709|rec2020|rec2100})` — T: color3|color4
- `vec3 hextilednormalmap(filename file = "", vec3 default = (0.5, 0.5, 1.0), vec2 texcoord = <UV0>, vec2 tiling = 1.0, float rotation = 1.0, vec2 rotationrange = (0.0, 360.0), float scale = 1.0, vec2 scalerange = (0.5, 2.0), float offset = 1.0, vec2 offsetrange = (0.0, 1.0), float falloff = 0.5, float strength = 1.0, bool flip_g = false, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, vec3 bitangent = <Bworld>)`
- `T hsvadjust<T>(T in = 0.0, vec3 amount = (0.0, 1.0, 1.0))` — T: color3|color4
- `T hsvtorgb<T>(T in = 0.0)` — T: color3|color4
- `T ifequal<T>(float value1 = 0.0, float value2 = 0.0, T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T ifequal(int value1 = 0, int value2 = 0, T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T ifequal(bool value1 = false, bool value2 = false, T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `bool ifequal(V value1, V value2)` — V: int|float
- `bool ifequal(bool value1 = false, bool value2 = false)`
- `T ifgreater<T>(float value1 = 1.0, float value2 = 0.0, T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T ifgreater(int value1 = 1, int value2 = 0, T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `bool ifgreater(V value1, V value2)` — V: int|float
- `T ifgreatereq<T>(float value1 = 1.0, float value2 = 0.0, T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T ifgreatereq(int value1 = 1, int value2 = 0, T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `bool ifgreatereq(V value1, V value2)` — V: int|float
- `T image<T>(filename file = "", string layer = "", T default = 0.0, vec2 texcoord = <UV0>, string uaddressmode = "periodic" {constant|clamp|periodic|mirror}, string vaddressmode = "periodic" {constant|clamp|periodic|mirror}, string filtertype = "linear" {closest|linear|cubic}, string framerange = "", int frameoffset = 0, string frameendaction = "constant" {constant|clamp|periodic|mirror})` — T: float|vec2|vec3|vec4|color3|color4
- `color4 in(color4 fg = 0.0, color4 bg = 0.0, float mix = 1.0)`
- `T inside<T>(T in = 0.0, float mask = 1.0)` — T: float|color3|color4
- `T invert<T>(T in = 0.0, T amount = 1.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T invert(T in = 0.0, float amount = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `T invertmatrix<T>(T in)` — T: mat3|mat4
- `color3 latlongimage(filename file = "", color3 default = 0.0, vec3 viewdir = (0.0, 0.0, 1.0), float rotation = 0.0)`
- `float line(vec2 texcoord = <UV0>, vec2 center = 0, float radius = 0.1, vec2 point1 = 0.25, vec2 point2 = 0.75)`
- `T ln<T>(T in = 1.0)` — T: float|vec2|vec3|vec4
- `T luminance<T>(T in = 0.0, color3 lumacoeffs = (0.2722287, 0.6740818, 0.0536895) {acescg|rec709|rec2020|rec2100})` — T: color3|color4
- `float magnitude<V>(V in = 0.0)` — V: vec2|vec3|vec4
- `color4 mask(color4 fg = 0.0, color4 bg = 0.0, float mix = 1.0)`
- `color4 matte(color4 fg = 0.0, color4 bg = 0.0, float mix = 1.0)`
- `T max<T>(T in1 = 0.0, T in2 = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T max(T in1 = 0.0, float in2 = 0.0)` — T: vec2|vec3|vec4|color3|color4
- `T min<T>(T in1 = 0.0, T in2 = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T min(T in1 = 0.0, float in2 = 0.0)` — T: vec2|vec3|vec4|color3|color4
- `T minus<T>(T fg = 0.0, T bg = 0.0, float mix = 1.0)` — T: float|color3|color4
- `T mix(T fg, T bg, float mix = 0.0)` — T: BSDF|EDF|VDF
- `T mix<T>(T fg = 0.0, T bg = 0.0, T mix = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T mix<T>(T fg, T bg, float mix = 0.0)` — T: vec2|vec3|vec4|color3|color4|displacementshader|surfaceshader|volumeshader
- `T modulo<T>(T in1 = 0.0, T in2 = 1.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T modulo(T in1 = 0.0, float in2 = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `T multiply(T in1, color3 in2 = 1.0)` — T: BSDF|EDF|VDF
- `T multiply(T in1, float in2 = 1.0)` — T: vec2|vec3|vec4|color3|color4|BSDF|EDF|VDF
- `T multiply<T>(T in1, T in2)` — T: float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T noise2d<T>(T amplitude = 1.0, float pivot = 0.0, vec2 texcoord = <UV0>)` — T: float|vec2|vec3|vec4
- `color3 noise2d<color3>(vec3 amplitude = 1.0, float pivot = 0.0, vec2 texcoord = <UV0>)`
- `color4 noise2d<color4>(vec4 amplitude = 1.0, float pivot = 0.0, vec2 texcoord = <UV0>)`
- `T noise2d(float amplitude = 1.0, float pivot = 0.0, vec2 texcoord = <UV0>)` — T: vec2|vec3|vec4|color3|color4
- `T noise3d<T>(T amplitude = 1.0, float pivot = 0.0, vec3 position = <Pobject>)` — T: float|vec2|vec3|vec4
- `color3 noise3d<color3>(vec3 amplitude = 1.0, float pivot = 0.0, vec3 position = <Pobject>)`
- `color4 noise3d<color4>(vec4 amplitude = 1.0, float pivot = 0.0, vec3 position = <Pobject>)`
- `T noise3d(float amplitude = 1.0, float pivot = 0.0, vec3 position = <Pobject>)` — T: vec2|vec3|vec4|color3|color4
- `vec3 normal(string space = "object" {model|object|world})`
- `T normalize<T>(T in = 0.0)` — T: vec2|vec3|vec4
- `vec3 normalmap<V>(vec3 in = (0.5, 0.5, 1.0), V scale = 1.0, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, vec3 bitangent = <Bworld>)` — V: float|vec2
- `bool not(bool in = false)`
- `bool or(bool in1 = false, bool in2 = false)`
- `material osl_set_ci(int output_mode, surfaceshader input_surfaceshader, material input_material, BSDF input_BSDF, EDF input_EDF, float input_float, color3 input_color3, color4 input_color4, vec2 input_vector2, vec3 input_vector3, vec4 input_vector4, int input_integer, bool input_boolean, mat3 input_matrix33, mat4 input_matrix44, displacementshader input_displacementshader)`
- `color4 out(color4 fg = 0.0, color4 bg = 0.0, float mix = 1.0)`
- `T outside<T>(T in = 0.0, float mask = 0.0)` — T: float|color3|color4
- `color4 over(color4 fg = 0.0, color4 bg = 0.0, float mix = 1.0)`
- `T overlay<T>(T fg = 0.0, T bg = 0.0, float mix = 1.0)` — T: float|color3|color4
- `vec2 place2d(vec2 texcoord = 0.0, vec2 pivot = 0.0, vec2 scale = 1.0, float rotate = 0.0, vec2 offset = 0.0, int operationorder = 0 {SRT|TRS})`
- `T plus<T>(T fg = 0.0, T bg = 0.0, float mix = 1.0)` — T: float|color3|color4
- `vec3 position(string space = "object" {model|object|world})`
- `T power<T>(T in1 = 0.0, T in2 = 1.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T power(T in1 = 0.0, float in2 = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `color4 premult(color4 in = (0.0, 0.0, 0.0, 1.0))`
- `color4 ramp(vec2 texcoord = <UV0>, int type = 0 {standard|radial|circular|box}, int interpolation = 1 {linear|smooth|step}, int num_intervals = 2, float interval1 = 0, color4 color1 = (0, 0, 0, 1), float interval2 = 1, color4 color2 = 1, float interval3 = 1, color4 color3 = 1, float interval4 = 1, color4 color4 = 1, float interval5 = 1, color4 color5 = 1, float interval6 = 1, color4 color6 = 1, float interval7 = 1, color4 color7 = 1, float interval8 = 1, color4 color8 = 1, float interval9 = 1, color4 color9 = 1, float interval10 = 1, color4 color10 = 1)`
- `T ramp4<T>(T valuetl = 0.0, T valuetr = 0.0, T valuebl = 0.0, T valuebr = 0.0, vec2 texcoord = <UV0>)` — T: float|vec2|vec3|vec4|color3|color4
- `color4 ramp_gradient(float x = 0, float interval1 = 0, float interval2 = 1, color4 color1 = (0, 0, 0, 1), color4 color2 = 1, int interpolation = 1 {linear|smooth|step}, color4 prev_color = (0, 0, 0, 1), int interval_num = 1, int num_intervals = 2)`
- `T ramplr<T>(T valuel = 0.0, T valuer = 0.0, vec2 texcoord = <UV0>)` — T: float|vec2|vec3|vec4|color3|color4
- `T ramptb<T>(T valuet = 0.0, T valueb = 0.0, vec2 texcoord = <UV0>)` — T: float|vec2|vec3|vec4|color3|color4
- `color3 randomcolor<V>(V in, float huelow = 0, float huehigh = 1, float saturationlow = 0.825, float saturationhigh = 1, float brightnesslow = 1, float brightnesshigh = 1, int seed = 0)` — V: int|float
- `float randomfloat<float>(float in = 0.0, float min = 0.0, float max = 1.0, int seed = 0)`
- `float randomfloat<int>(int in = 0, float min = 0.0, float max = 1.0, int seed = 0)`
- `T range<T>(T in = 0.0, T inlow = 0.0, T inhigh = 1.0, T gamma = 1.0, T outlow = 0.0, T outhigh = 1.0, bool doclamp = false)` — T: float|vec2|vec3|vec4|color3|color4
- `T range(T in = 0.0, float inlow = 0.0, float inhigh = 1.0, float gamma = 1.0, float outlow = 0.0, float outhigh = 1.0, bool doclamp = false)` — T: vec2|vec3|vec4|color3|color4
- `vec3 reflect(vec3 in = (1.0, 0.0, 0.0), vec3 normal = <Nworld>)`
- `vec3 refract(vec3 in = (1.0, 0.0, 0.0), vec3 normal = <Nworld>, float ior = 1.0)`
- `T remap<T>(T in = 0.0, T inlow = 0.0, T inhigh = 1.0, T outlow = 0.0, T outhigh = 1.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T remap(T in = 0.0, float inlow = 0.0, float inhigh = 1.0, float outlow = 0.0, float outhigh = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `T rgbtohsv<T>(T in = 0.0)` — T: color3|color4
- `vec2 rotate2d(vec2 in = 0.0, float amount = 0.0)`
- `vec3 rotate3d(vec3 in = 0.0, float amount = 0.0, vec3 axis = (0.0, 1.0, 0.0))`
- `T round<T>(T in = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `int round<int>(float in = 0.0)`
- `T safepower<T>(T in1 = 0.0, T in2 = 1.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T safepower(T in1 = 0.0, float in2 = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `T saturate<T>(T in = 0.0, float amount = 1.0, color3 lumacoeffs = (0.2722287, 0.6740818, 0.0536895) {acescg|rec709|rec2020|rec2100})` — T: color3|color4
- `T screen<T>(T fg = 0.0, T bg = 0.0, float mix = 1.0)` — T: float|color3|color4
- `{float outx, float outy} separate2(vec2 in = 0.0)`
- `{float outr, float outg, float outb} separate3<color3>(color3 in = 0.0)`
- `{float outx, float outy, float outz} separate3<vec3>(vec3 in = 0.0)`
- `{float outr, float outg, float outb, float outa} separate4<color4>(color4 in = 0.0)`
- `{float outx, float outy, float outz, float outw} separate4<vec4>(vec4 in = 0.0)`
- `T sign<T>(T in = 0.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T sin<T>(T in = 0.0)` — T: float|vec2|vec3|vec4
- `T smoothstep<T>(T in = 0.0, T low = 0.0, T high = 1.0)` — T: float|vec2|vec3|vec4|color3|color4
- `T smoothstep(T in = 0.0, float low = 0.0, float high = 1.0)` — T: vec2|vec3|vec4|color3|color4
- `T splitlr<T>(T valuel = 0.0, T valuer = 0.0, float center = 0.5, vec2 texcoord = <UV0>)` — T: float|vec2|vec3|vec4|color3|color4
- `T splittb<T>(T valuet = 0.0, T valueb = 0.0, float center = 0.5, vec2 texcoord = <UV0>)` — T: float|vec2|vec3|vec4|color3|color4
- `T sqrt<T>(T in = 0.0)` — T: float|vec2|vec3|vec4
- `T subtract<T>(T in1, T in2)` — T: int|float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T subtract(T in1, float in2 = 0.0)` — T: vec2|vec3|vec4|color3|color4|mat3|mat4
- `surfaceshader surface_unlit(float emission = 1.0, color3 emission_color = 1, float transmission = 0.0, color3 transmission_color = 1, float opacity = 1.0)`
- `material surfacematerial(surfaceshader surfaceshader, surfaceshader backsurfaceshader, displacementshader displacementshader)`
- `T switch<T>(T in1 = 0.0, T in2 = 0.0, T in3 = 0.0, T in4 = 0.0, T in5 = 0.0, T in6 = 0.0, T in7 = 0.0, T in8 = 0.0, T in9 = 0.0, T in10 = 0.0, float which = 0.0)` — T: float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T switch(T in1 = 0.0, T in2 = 0.0, T in3 = 0.0, T in4 = 0.0, T in5 = 0.0, T in6 = 0.0, T in7 = 0.0, T in8 = 0.0, T in9 = 0.0, T in10 = 0.0, int which = 0)` — T: float|vec2|vec3|vec4|color3|color4|mat3|mat4
- `T tan<T>(T in = 0.0)` — T: float|vec2|vec3|vec4
- `vec3 tangent(string space = "object" {model|object|world}, int index = 0)`
- `T texcoord<T>(int index = 0)` — T: vec2|vec3
- `color3 tiledcircles(vec2 texcoord = <UV0>, vec2 uvtiling = 1.0, vec2 uvoffset = 0.0, float size = 0.5, bool staggered = false)`
- `color3 tiledcloverleafs(vec2 texcoord = <UV0>, vec2 uvtiling = 1.0, vec2 uvoffset = 0.0, float size = 0.5, bool staggered = false)`
- `color3 tiledhexagons(vec2 texcoord = <UV0>, vec2 uvtiling = 1.0, vec2 uvoffset = 0.0, float size = 0.5, bool staggered = false)`
- `T tiledimage<T>(filename file = "", T default = 0.0, vec2 texcoord = <UV0>, vec2 uvtiling = 1.0, vec2 uvoffset = 0.0, vec2 realworldimagesize = 1.0, vec2 realworldtilesize = 1.0, string filtertype = "linear" {closest|linear|cubic}, string framerange = "", int frameoffset = 0, string frameendaction = "constant" {constant|clamp|periodic|mirror})` — T: float|vec2|vec3|vec4|color3|color4
- `float time(float fps = 24.0)`
- `vec2 transformmatrix(vec2 in = 0.0, mat3 mat = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))`
- `vec3 transformmatrix<vec3>(vec3 in = 0.0, mat3 mat = (1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0))`
- `vec3 transformmatrix(vec3 in = 0.0, mat4 mat = (1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0))`
- `vec4 transformmatrix<vec4>(vec4 in = 0.0, mat4 mat = (1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0))`
- `vec3 transformnormal(vec3 in = (0.0, 0.0, 1.0), string fromspace = "", string tospace = "")`
- `vec3 transformpoint(vec3 in = 0.0, string fromspace = "", string tospace = "")`
- `vec3 transformvector(vec3 in = 0.0, string fromspace = "", string tospace = "")`
- `T transpose<T>(T in)` — T: mat3|mat4
- `float trianglewave(float in = 0)`
- `T triplanarprojection<T>(filename filex = "", filename filey = "", filename filez = "", string layerx = "", string layery = "", string layerz = "", T default = 0.0, vec3 position = <Pobject>, vec3 normal = <Nobject>, int upaxis = 2 {X|Y|Z}, float blend = 1.0, string filtertype = "linear" {closest|linear|cubic}, string framerange = "", int frameoffset = 0, string frameendaction = "constant" {constant|clamp|periodic|mirror})` — T: float|vec2|vec3|vec4|color3|color4
- `float unifiednoise2d(vec2 texcoord = <UV0>, vec2 freq = 1, vec2 offset = 0, float jitter = 1, float outmin = 0, float outmax = 1, bool clampoutput = true, int octaves = 3, float lacunarity = 2, float diminish = 0.5, int type = 0 {Perlin|Cell|Worley|Fractal}, int style = 0 {Distance|Solid})`
- `float unifiednoise3d(vec3 position = <Pobject>, vec3 freq = 1, vec3 offset = 0, float jitter = 1, float outmin = 0, float outmax = 1, bool clampoutput = true, int octaves = 3, float lacunarity = 2, float diminish = 0.5, int type = 0 {Perlin|Cell|Worley|Fractal}, int style = 0 {Distance|Solid})`
- `color4 unpremult(color4 in = (0.0, 0.0, 0.0, 1.0))`
- `material volumematerial(volumeshader volumeshader)`
- `T worleynoise2d<T>(vec2 texcoord = <UV0>, float jitter = 1.0, int style = 0 {Distance|Solid})` — T: float|vec2|vec3
- `T worleynoise3d<T>(vec3 position = <Pobject>, float jitter = 1.0, int style = 0 {Distance|Solid})` — T: float|vec2|vec3
- `bool xor(bool in1 = false, bool in2 = false)`

## pbrlib

- `VDF absorption_vdf(vec3 absorption = 0.0)`
- `VDF anisotropic_vdf(vec3 absorption = 0.0, vec3 scattering = 0.0, float anisotropy = 0.0)`
- `{color3 ior, color3 extinction} artistic_ior(color3 reflectivity = (0.944, 0.776, 0.373), color3 edge_color = (0.998, 0.981, 0.751))`
- `color3 blackbody(float temperature = 5000.0)`
- `BSDF burley_diffuse_bsdf(float weight = 1.0, color3 color = 0.18, float roughness = 0.0, vec3 normal = <Nworld>)`
- `vec3 chiang_hair_absorption_from_color(color3 color = 1.0, float azimuthal_roughness = 0.2)`
- `BSDF chiang_hair_bsdf(color3 tint_R = 1, color3 tint_TT = 1, color3 tint_TRT = 1, float ior = 1.55, vec2 roughness_R = 0.1, vec2 roughness_TT = 0.05, vec2 roughness_TRT = 0.2, float cuticle_angle = 0.5, vec3 absorption_coefficient = 0.0, vec3 normal = <Nworld>, vec3 curve_direction = <Tworld>)`
- `{vec2 roughness_R, vec2 roughness_TT, vec2 roughness_TRT} chiang_hair_roughness(float longitudinal = 0.1, float azimuthal = 0.2, float scale_TT = 0.5, float scale_TRT = 2.0)`
- `BSDF conductor_bsdf(float weight = 1.0, color3 ior = (0.183, 0.421, 1.373), color3 extinction = (3.424, 2.346, 1.770), vec2 roughness = 0.05, bool retroreflective = false, float thinfilm_thickness = 0, float thinfilm_ior = 1.5, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, string distribution = "ggx" {ggx})`
- `EDF conical_edf(color3 color = 1.0, vec3 normal = <Nworld>, float inner_angle = 60.0, float outer_angle = 0.0)`
- `vec3 deon_hair_absorption_from_melanin(float melanin_concentration = 0.25, float melanin_redness = 0.5, color3 eumelanin_color = (0.657704, 0.498077, 0.254107), color3 pheomelanin_color = (0.829444, 0.67032, 0.349938))`
- `BSDF dielectric_bsdf(float weight = 1.0, color3 tint = 1.0, float ior = 1.5, vec2 roughness = 0.05, bool retroreflective = false, float thinfilm_thickness = 0, float thinfilm_ior = 1.5, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, string distribution = "ggx" {ggx}, string scatter_mode = "R" {R|T|RT})`
- `displacementshader displacement<V>(V displacement = 0.0, float scale = 1.0)` — V: float|vec3
- `BSDF generalized_schlick_bsdf(float weight = 1.0, color3 color0 = 1.0, color3 color82 = 1.0, color3 color90 = 1.0, float exponent = 5.0, vec2 roughness = 0.05, bool retroreflective = false, float thinfilm_thickness = 0, float thinfilm_ior = 1.5, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, string distribution = "ggx" {ggx}, string scatter_mode = "R" {R|T|RT})`
- `EDF generalized_schlick_edf(color3 color0 = 1.0, color3 color90 = 1.0, float exponent = 5.0, EDF base)`
- `vec2 glossiness_anisotropy(float glossiness = 1.0, float anisotropy = 0.0)`
- `BSDF layer(BSDF top, BSDF base)`
- `BSDF layer(BSDF top, VDF base)`
- `lightshader light(EDF edf, float intensity = 1.0, float exposure = 0.0)`
- `EDF measured_edf(color3 color = 1.0, vec3 normal = <Nworld>, filename file = "")`
- `BSDF oren_nayar_diffuse_bsdf(float weight = 1.0, color3 color = 0.18, float roughness = 0.0, vec3 normal = <Nworld>, bool energy_compensation = false)`
- `vec2 roughness_anisotropy(float roughness = 0.0, float anisotropy = 0.0)`
- `vec2 roughness_dual(vec2 roughness = 0.0)`
- `BSDF sheen_bsdf(float weight = 1.0, color3 color = 1.0, float roughness = 0.3, vec3 normal = <Nworld>, string mode = "conty_kulla" {conty_kulla|zeltner})`
- `BSDF subsurface_bsdf(float weight = 1.0, color3 color = 0.18, color3 radius = 1.0, float anisotropy = 0.0, vec3 normal = <Nworld>)`
- `surfaceshader surface(BSDF bsdf, EDF edf, float opacity = 1.0, bool thin_walled = false)`
- `BSDF translucent_bsdf(float weight = 1.0, color3 color = 1.0, vec3 normal = <Nworld>)`
- `EDF uniform_edf(color3 color = 1.0)`
- `volumeshader volume(VDF vdf, EDF edf)`

## bxdf

- `T LamaAdd(T material1, T material2, float weight1 = 1.0, float weight2 = 0.0)` — T: BSDF|EDF
- `BSDF LamaConductor(color3 tint = 1, int fresnelMode = 0 {Artistic|Scientific}, vec3 IOR = (0.180000007153, 0.419999986887, 1.37000000477), vec3 extinction = (3.42000007629, 2.34999990463, 1.76999998093), color3 reflectivity = (0.9450, 0.7772, 0.3737), color3 edgeColor = (0.7137, 0.7373, 0.4550), float roughness = 0.1, vec3 normal = <Nworld>, float anisotropy = 0.0, vec3 anisotropyDirection = <Tworld>, float anisotropyRotation = 0.0)`
- `BSDF LamaDielectric(color3 reflectionTint = 1.0, color3 transmissionTint = 1.0, int fresnelMode = 0 {Artistic|Scientific}, float IOR = 1.5, float reflectivity = 0.04, float roughness = 0.1, vec3 normal = <Nworld>, float anisotropy = 0.0, vec3 direction = <Tworld>, float rotation = 0.0, color3 absorptionColor = 1.0, float absorptionRadius = 1.0, color3 scatterColor = 0.0, float scatterAnisotropy = 0.0)`
- `BSDF LamaDiffuse(color3 color = 0.18, float roughness = 0.0, vec3 normal = <Nworld>, float energyCompensation = 1.0)`
- `EDF LamaEmission(color3 color = 0.0)`
- `BSDF LamaGeneralizedSchlick(color3 reflectionTint = 1.0, color3 transmissionTint = 1.0, int fresnelMode = 0 {Artistic|Scientific}, vec3 IOR = 1.5, color3 reflectivity = 0.04, color3 reflectivity90 = 1.0, float reflectivityProfile = 0.2, float roughness = 0.1, vec3 normal = <Nworld>, float anisotropy = -1.0, vec3 anisotropyDirection = <Tworld>, float anisotropyRotation = 0.0, color3 absorptionColor = 1.0, float absorptionRadius = 1.0, color3 scatterColor = 0.0, float scatterAnisotropy = 0.0)`
- `BSDF LamaIridescence(float roughness = 0.01, float anisotropy = 0.0, vec3 anisotropyDirection = <Tworld>, float anisotropyRotation = 0.0, float relativeFilmThickness = 0.5, float minFilmThickness = 400.0, float maxFilmThickness = 800.0, float filmIOR = 1.3)`
- `BSDF LamaLayer(BSDF materialTop, BSDF materialBase, float topMix = 1.0, float topThickness = 0.0)`
- `T LamaMix(T material1, T material2, float mix = 0.0)` — T: BSDF|EDF
- `BSDF LamaSSS(color3 color = 0.18, vec3 normal = <Nworld>, color3 sssRadius = 0.0, float sssScale = 1.0, int sssMode = 0 {Path-traced Davis|Path-traced exponential|Diffusion Burley|Diffusion Burley (mean free path)}, float sssIOR = 1.0, float sssAnisotropy = 0.0, float sssBleed = 0.0, float sssFollowTopology = 0.0, string sssSubset = "", int sssContinuationRays = 0, float sssUnitLength = 0.00328, int mode = 0 {Reflection|Transmission|Reflection(with direct illumination)}, int albedoInversionMethod = 0 {Pixar|Chiang})`
- `BSDF LamaSheen(color3 color = 1, float roughness = 0.1, vec3 normal = <Nworld>)`
- `material LamaSurface(BSDF materialFront, BSDF materialBack, float presence = 1.0)`
- `BSDF LamaTranslucent(color3 color = 0.18, float roughness = 0.0, vec3 normal = <Nworld>, float energyCompensation = 1.0)`
- `surfaceshader UsdPreviewSurface(color3 diffuseColor = 0.18, color3 emissiveColor = 0, int useSpecularWorkflow = 0, color3 specularColor = 0, float metallic = 0, float roughness = 0.5, float clearcoat = 0, float clearcoatRoughness = 0.01, float opacity = 1, int opacityMode = 0 {transparent|presence}, float opacityThreshold = 0, float ior = 1.5, vec3 normal = (0, 0, 1), float displacement = 0, float occlusion = 1)`
- `T UsdPrimvarReader<T>(string varname = "", T fallback)` — T: bool|int|float|vec2|vec3|vec4|string|filename
- `vec2 UsdTransform2d(vec2 in = 0, float rotation = 0, vec2 scale = 1, vec2 translation = 0)`
- `{float r, float g, float b, float a, color3 rgb} UsdUVTexture(filename file = "", vec2 st = <UV0>, string wrapS = "periodic" {black|clamp|periodic|mirror}, string wrapT = "periodic" {black|clamp|periodic|mirror}, color4 fallback = (0, 0, 0, 1), color4 scale = 1, color4 bias = 0)`
- `surfaceshader disney_principled(color3 baseColor = 0.16, float metallic = 0.0, float roughness = 0.5, float anisotropic = 0.0, float specular = 0.5, float specularTint = 0, float sheen = 0.0, float sheenTint = 0.5, float clearcoat = 0.0, float clearcoatGloss = 1.0, float specTrans = 0.0, float ior = 1.5, float subsurface = 0.0, color3 subsurfaceDistance = 1.0)`
- `{float anisotropy_strength_out, float anisotropy_rotation_out} gltf_anisotropy_image(filename file = "", vec3 default = (1.0, 0.5, 1), vec2 texcoord = <UV0>, vec2 pivot = (0, 1), vec2 scale = 1, float rotate = 0, vec2 offset = 0, int operationorder = 0, string uaddressmode = "periodic" {constant|clamp|periodic|mirror}, string vaddressmode = "periodic" {constant|clamp|periodic|mirror}, string filtertype = "linear" {closest|linear|cubic}, float anisotropy_strength = 1, float anisotropy_rotation = 0)`
- `{color3 outcolor, float outa} gltf_colorimage(filename file = "", color4 default = 0, vec2 texcoord = <UV0>, vec2 pivot = (0, 1), vec2 scale = 1, float rotate = 0, vec2 offset = 0, int operationorder = 1, string uaddressmode = "periodic" {constant|clamp|periodic|mirror}, string vaddressmode = "periodic" {constant|clamp|periodic|mirror}, string filtertype = "linear" {closest|linear|cubic}, color4 color = 1, color4 geomcolor = 1)`
- `T gltf_image(filename file = "", T factor = 1, T default = 0, vec2 texcoord = <UV0>, vec2 pivot = (0, 1), vec2 scale = 1, float rotate = 0, vec2 offset = 0, int operationorder, string uaddressmode = "periodic" {constant|clamp|periodic|mirror}, string vaddressmode = "periodic" {constant|clamp|periodic|mirror}, string filtertype = "linear" {closest|linear|cubic})` — T: float|color3|color4
- `vec3 gltf_image(filename file = "", vec3 default = 0, vec2 texcoord = <UV0>, vec2 pivot = (0, 1), vec2 scale = 1, float rotate = 0, vec2 offset = 0, int operationorder = 0, string uaddressmode = "periodic" {constant|clamp|periodic|mirror}, string vaddressmode = "periodic" {constant|clamp|periodic|mirror}, string filtertype = "linear" {closest|linear|cubic})`
- `float gltf_iridescence_thickness(filename file = "", vec3 default = 0, vec2 texcoord = <UV0>, vec2 pivot = 0, vec2 scale = 1, float rotate = 0, vec2 offset = 0, string uaddressmode = "periodic" {constant|clamp|periodic|mirror}, string vaddressmode = "periodic" {constant|clamp|periodic|mirror}, string filtertype = "linear" {closest|linear|cubic}, float thicknessMin = 100, float thicknessMax = 400)`
- `vec3 gltf_normalmap(filename file = "", vec3 default = (0.5, 0.5, 1), vec2 texcoord = <UV0>, vec2 pivot = (0, 1), vec2 scale = 1, float rotate = 0, vec2 offset = 0, int operationorder = 0, string uaddressmode = "periodic" {constant|clamp|periodic|mirror}, string vaddressmode = "periodic" {constant|clamp|periodic|mirror}, string filtertype = "linear" {closest|linear|cubic})`
- `surfaceshader gltf_pbr(color3 base_color = 1, float metallic = 1, float roughness = 1, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, float occlusion = 1, float transmission = 0, float specular = 1, color3 specular_color = 1, float ior = 1.5, float alpha = 1, int alpha_mode = 0 {OPAQUE|MASK|BLEND}, float alpha_cutoff = 0.5, float iridescence = 0, float iridescence_ior = 1.3, float iridescence_thickness = 100, color3 sheen_color = 0, float sheen_roughness = 0, float clearcoat = 0, float clearcoat_roughness = 0, vec3 clearcoat_normal = <Nworld>, color3 emissive = 0, float emissive_strength = 1, float thickness = 0, float attenuation_distance, color3 attenuation_color = 1, float anisotropy_strength = 0, float anisotropy_rotation = 0, float dispersion = 0)`
- `vec2 open_pbr_anisotropy(float roughness = 0.0, float anisotropy = 0.0)`
- `surfaceshader open_pbr_surface(float base_weight = 1.0, color3 base_color = 0.8, float base_diffuse_roughness = 0.0, float base_metalness = 0.0, float specular_weight = 1.0, color3 specular_color = 1, float specular_roughness = 0.3, float specular_ior = 1.5, float specular_roughness_anisotropy = 0.0, float transmission_weight = 0.0, color3 transmission_color = 1, float transmission_depth = 0.0, color3 transmission_scatter = 0, float transmission_scatter_anisotropy = 0.0, float transmission_dispersion_scale = 0.0, float transmission_dispersion_abbe_number = 20.0, float subsurface_weight = 0, color3 subsurface_color = 0.8, float subsurface_radius = 1.0, color3 subsurface_radius_scale = (1.0, 0.5, 0.25), float subsurface_scatter_anisotropy = 0.0, float fuzz_weight = 0.0, color3 fuzz_color = 1, float fuzz_roughness = 0.5, float coat_weight = 0.0, color3 coat_color = 1, float coat_roughness = 0.0, float coat_roughness_anisotropy = 0.0, float coat_ior = 1.6, float coat_darkening = 1.0, float thin_film_weight = 0, float thin_film_thickness = 0.5, float thin_film_ior = 1.4, float emission_luminance = 0.0, color3 emission_color = 1, float geometry_opacity = 1, bool geometry_thin_walled = false, vec3 geometry_normal = <Nworld>, vec3 geometry_coat_normal = <Nworld>, vec3 geometry_tangent = <Tworld>, vec3 geometry_coat_tangent = <Tworld>)`
- `{float base_out, color3 base_color_out, float diffuse_roughness_out, float metalness_out, float specular_out, color3 specular_color_out, float specular_roughness_out, float specular_IOR_out, float specular_anisotropy_out, float transmission_out, color3 transmission_color_out, float transmission_depth_out, color3 transmission_scatter_out, float transmission_scatter_anisotropy_out, float transmission_dispersion_out, float subsurface_out, color3 subsurface_color_out, color3 subsurface_radius_out, float subsurface_scale_out, float subsurface_anisotropy_out, float sheen_out, color3 sheen_color_out, float sheen_roughness_out, float coat_out, color3 coat_color_out, float coat_roughness_out, float coat_anisotropy_out, float coat_IOR_out, float coat_affect_roughness_out, float thin_film_thickness_out, float thin_film_IOR_out, float emission_out, color3 emission_color_out, color3 opacity_out, bool thin_walled_out} open_pbr_surface_to_standard_surface(float base_weight = 1.0, color3 base_color = 0.8, float base_diffuse_roughness = 0.0, float base_metalness = 0.0, float specular_weight = 1.0, color3 specular_color = 1, float specular_roughness = 0.3, float specular_ior = 1.5, float specular_roughness_anisotropy = 0.0, float transmission_weight = 0.0, color3 transmission_color = 1, float transmission_depth = 0.0, color3 transmission_scatter = 0, float transmission_scatter_anisotropy = 0.0, float transmission_dispersion_scale = 0.0, float subsurface_weight = 0, color3 subsurface_color = 0.8, float subsurface_radius = 1.0, color3 subsurface_radius_scale = (1.0, 0.5, 0.25), float subsurface_scatter_anisotropy = 0.0, float fuzz_weight = 0.0, color3 fuzz_color = 1, float fuzz_roughness = 0.5, float coat_weight = 0.0, color3 coat_color = 1, float coat_roughness = 0.0, float coat_roughness_anisotropy = 0.0, float coat_ior = 1.6, float coat_darkening = 1.0, float thin_film_weight = 0, float thin_film_thickness = 0.5, float thin_film_ior = 1.4, float emission_luminance = 0.0, color3 emission_color = 1, float geometry_opacity = 1, bool geometry_thin_walled = false)`
- `surfaceshader standard_surface(float base = 1.0, color3 base_color = 0.8, float diffuse_roughness = 0, float metalness = 0, float specular = 1, color3 specular_color = 1, float specular_roughness = 0.2, float specular_IOR = 1.5, float specular_anisotropy = 0, float specular_rotation = 0, float transmission = 0, color3 transmission_color = 1, float transmission_depth = 0, color3 transmission_scatter = 0, float transmission_scatter_anisotropy = 0, float transmission_dispersion = 0, float transmission_extra_roughness = 0, float subsurface = 0, color3 subsurface_color = 1, color3 subsurface_radius = 1, float subsurface_scale = 1, float subsurface_anisotropy = 0, float sheen = 0, color3 sheen_color = 1, float sheen_roughness = 0.3, float coat = 0, color3 coat_color = 1, float coat_roughness = 0.1, float coat_anisotropy = 0.0, float coat_rotation = 0.0, float coat_IOR = 1.5, vec3 coat_normal = <Nworld>, float coat_affect_color = 0, float coat_affect_roughness = 0, float thin_film_thickness = 0, float thin_film_IOR = 1.5, float emission = 0, color3 emission_color = 1, color3 opacity = 1, bool thin_walled = false, vec3 normal = <Nworld>, vec3 tangent = <Tworld>)`
- `{color3 diffuseColor_out, color3 emissiveColor_out, float metallic_out, float roughness_out, float clearcoat_out, float clearcoatRoughness_out, float opacity_out, float ior_out, vec3 normal_out} standard_surface_to_UsdPreviewSurface(float metalness = 0, float base = 1, color3 base_color = 0.8, float specular = 1, color3 specular_color = 1, float specular_IOR = 1.5, float specular_roughness = 0.2, float coat = 0, color3 coat_color = 1, float coat_roughness = 0.1, float subsurface = 0, color3 subsurface_color = 1, float emission = 0, color3 emission_color = 1, color3 opacity = 1, vec3 normal = (0.5, 0.5, 1.0))`
- `{color3 base_color_out, float metallic_out, float roughness_out, float specular_out, color3 specular_color_out, float ior_out, float anisotropy_strength_out, float anisotropy_rotation_out, float transmission_out, float thickness_out, color3 attenuation_color_out, float attenuation_distance_out, color3 sheen_color_out, float sheen_roughness_out, float clearcoat_out, float clearcoat_roughness_out, float iridescence_out, float iridescence_ior_out, float iridescence_thickness_out, float dispersion_out, color3 emissive_out, float emissive_strength_out, vec3 normal_out, vec3 tangent_out, float alpha_out, int alpha_mode_out} standard_surface_to_gltf_pbr(float base = 1, color3 base_color = 0.8, float metalness = 0, float specular = 1, color3 specular_color = 1, float specular_anisotropy = 0.0, float specular_rotation = 0.0, float specular_roughness = 0.2, float specular_IOR = 1.5, float transmission = 0, color3 transmission_color = 1, float transmission_depth = 0, float transmission_dispersion = 0, float sheen = 0, color3 sheen_color = 1, float sheen_roughness = 0.3, float coat = 0, color3 coat_color = 0, float coat_roughness = 0.1, float emission = 0, color3 emission_color = 1, vec3 normal = <Nworld>, vec3 tangent = <Tworld>, color3 opacity = 1, bool thin_walled = false, float thin_film_thickness = 0.0, float thin_film_IOR = 1.5)`
- `{float base_weight_out, color3 base_color_out, float base_diffuse_roughness_out, float base_metalness_out, float specular_weight_out, color3 specular_color_out, float specular_roughness_out, float specular_ior_out, float specular_roughness_anisotropy_out, float transmission_weight_out, color3 transmission_color_out, float transmission_depth_out, color3 transmission_scatter_out, float transmission_scatter_anisotropy_out, float transmission_dispersion_scale_out, float subsurface_weight_out, color3 subsurface_color_out, float subsurface_radius_out, color3 subsurface_radius_scale_out, float subsurface_scatter_anisotropy_out, float fuzz_weight_out, color3 fuzz_color_out, float fuzz_roughness_out, float coat_weight_out, color3 coat_color_out, float coat_roughness_out, float coat_roughness_anisotropy_out, float coat_ior_out, float coat_darkening_out, float thin_film_weight_out, float thin_film_thickness_out, float thin_film_ior_out, float emission_luminance_out, color3 emission_color_out, float geometry_opacity_out, bool geometry_thin_walled_out} standard_surface_to_open_pbr_surface(float base = 0.8, color3 base_color = 1.0, float diffuse_roughness = 0, float metalness = 0, float specular = 1, color3 specular_color = 1, float specular_roughness = 0.2, float specular_IOR = 1.5, float specular_anisotropy = 0, float transmission = 0, color3 transmission_color = 1, float transmission_depth = 0, color3 transmission_scatter = 0, float transmission_scatter_anisotropy = 0, float transmission_dispersion = 0, float subsurface = 0, color3 subsurface_color = 1, color3 subsurface_radius = 1, float subsurface_scale = 1, float subsurface_anisotropy = 0, float sheen = 0, color3 sheen_color = 1, float sheen_roughness = 0.3, float coat = 0, color3 coat_color = 1, float coat_roughness = 0.1, float coat_anisotropy = 0.0, float coat_IOR = 1.5, float coat_affect_roughness = 0, float thin_film_thickness = 0, float thin_film_IOR = 1.5, float emission = 0, color3 emission_color = 1, color3 opacity = 1, bool thin_walled = false)`

## nprlib

- `float facingratio(vec3 viewdirection = <Vworld>, vec3 normal = <Nworld>, bool faceforward = true, bool invert = false)`
- `color3 gooch_shade(color3 warm_color = (0.8, 0.8, 0.7), color3 cool_color = (0.3, 0.3, 0.8), float specular_intensity = 1, float shininess = 64, vec3 light_direction = (1, -0.5, -0.5))`
- `vec3 viewdirection(string space = "world" {model|object|world})`

## lights

- `lightshader directional_light(vec3 direction, color3 color, float intensity)`
- `lightshader point_light(vec3 position, color3 color, float intensity, float decay_rate = 2.0)`
- `lightshader spot_light(vec3 position, vec3 direction, color3 color, float intensity, float decay_rate = 2.0, float inner_angle, float outer_angle)`

## cmlib

- `T acescg_to_lin_rec709<T>(T in)` — T: color3|color4
- `T adobergb_to_lin_rec709<T>(T in)` — T: color3|color4
- `T g18_rec709_to_lin_rec709<T>(T in)` — T: color3|color4
- `T g22_ap1_to_lin_rec709<T>(T in)` — T: color3|color4
- `T g22_rec709_to_lin_rec709<T>(T in)` — T: color3|color4
- `T lin_adobergb_to_lin_rec709<T>(T in)` — T: color3|color4
- `T lin_displayp3_to_lin_rec709<T>(T in)` — T: color3|color4
- `T rec709_display_to_lin_rec709<T>(T in)` — T: color3|color4
- `T srgb_displayp3_to_lin_rec709<T>(T in)` — T: color3|color4
- `T srgb_texture_to_lin_rec709<T>(T in)` — T: color3|color4
