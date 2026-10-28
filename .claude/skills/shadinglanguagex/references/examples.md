# Verified SLX examples

Each block compiles with mxslc++ (MaterialX 1.39.5). A first line of the form `// mxslc: <args>` shows
the extra command-line arguments it needs.

## 1. Basic material and port assignment

```slx
surfaceshader gold = standard_surface();
gold.base_color = color3{1.059, 0.773, 0.307};
gold.specular_color = color3{1.001, 0.985, 0.523};
gold.specular_roughness = 0.02;
gold.metalness = 1.0;
material gold_mat = surfacematerial(gold);
```

## 2. Randomly coloured tiles

```slx
float tiles = 10.0;
float u, v = texcoord() * tiles;
float seed = floor(u) + floor(v) * tiles;
surfacematerial(standard_surface(base_color = randomcolor(seed), specular_roughness = 1.0));
```

## 3. Toon shading with if-expressions

```slx
vec3 i = viewdirection("world");
vec3 n = normal("world");
float theta = dotproduct(-i, n);
color3 c = if (|theta| > 0.65) { color3{1.0, 0.2, 0.143} }
           else if (|theta| > 0.3) { color3{0.6, 0.1, 0.08} }
           else { color3{0.154, 1.0, 1.0} };
material toon = surfacematerial(surface_unlit(emission_color = c));
```

## 4. Fractal displacement with a loop

```slx
const vec2 uv = position().xy;
mutable float scale = 2.0;
mutable float height = 0.0;
for (int i from 0 to 3)
{
    float s2 = scale * scale;
    height += noise2d(texcoord = uv / s2) * s2;
    scale *= 2.0;
}
surfacematerial(
    standard_surface(base_color = color3{0.82, 0.796, 0.761}, specular_roughness = 1.0),
    null,                                   // backsurfaceshader left unset
    displacement(height, scale = 0.05)
);
```

## 5. Globals and conditional assignment in a loop

```slx
// mxslc: -g N 5 -g radius 0.1
global int N;
global float radius;

mutable color3 c = color3{1.0};
vec2 uv = texcoord();
for (int i from 1 to N)
{
    vec2 p = {randomfloat(i, seed = 0), randomfloat(i, seed = 1)};
    c = if (distance(p, uv) < radius) { randomcolor(i) };     // else: keep c
}
surfacematerial(standard_surface(base_color = c));
```

The Python equivalent is `mxslc.compile_file_to_file("circles.mxsl", mxslc.CompileOptions(globals={"N": 5, "radius": 0.1}))`.

## 6. Textures, colourspaces and normal maps

```slx
@file.colorspace "srgb_texture"
color3 albedo = image("textures/brick_albedo.png");
float rough = image("textures/brick_rough.png", default = 0.5);
vec3 nmap = image("textures/brick_normal.png");
vec2 tiled = texcoord() * 4.0;
color3 detail = tiledimage<color3>("textures/grain.png", texcoord = tiled);

surfaceshader s = open_pbr_surface(
    base_color = albedo * detail,
    specular_roughness = rough,
    geometry_normal = normalmap(nmap)
);
material brick = surfacematerial(s);
```

## 7. Structs, classes and methods

```slx
using Ray = {vec3 orig, vec3 dir};

class Sphere
{
    vec3 center;
    float radius;

    Sphere(vec3 center, float radius = 1.0)
    {
        this.center = center;
        this.radius = radius;
    }

    // distance along the ray to the first hit, or -1 when missed
    float hit(Ray r)
    {
        vec3 oc = r.orig - center;
        float b = dotproduct(oc, r.dir);
        float disc = b * b - (dotproduct(oc, oc) - radius * radius);
        return if (disc < 0.0) { -1.0 } else { -b - sqrt(disc) };
    }
}

vec2 uv = texcoord() * 2.0 - 1.0;
Ray ray = {vec3{0, 0, -3}, normalize(vec3{uv, 1.0})};
Sphere ball = Sphere{vec3{0}, 0.75};
float t = ball.hit(ray);
color3 c = if (t > 0.0) { color3{1.0 - t / 4.0} } else { color3{0.05} };
surfacematerial(surface_unlit(emission_color = c));
```

## 8. Templates, out-parameters and operator overloading

```slx
inline T saturate01<float, vec2, vec3, color3>(T x) { return clamp(x, 0.0, 1.0); }

inline void polar(vec2 p, out float r, out float a)
{
    r = magnitude(p);
    a = atan2(p.y, p.x);
}

using Complex = {float re, float im};
inline Complex __mul__(Complex a, Complex b) { return {a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re}; }
inline Complex __add__(Complex a, Complex b) { return {a.re + b.re, a.im + b.im}; }

vec2 p = texcoord() * 2.0 - 1.0;
polar(p, float r, float a);                    // declares r and a
mutable Complex z = {0.0, 0.0};
Complex k = {p.x, p.y};
for (int i from 1 to 6) { z = z * z + k; }     // a few Mandelbrot iterations, unrolled
float m = saturate01(z.re * z.re + z.im * z.im);
surfacematerial(surface_unlit(emission_color = saturate01(color3{m, r, a / 6.283})));
```

## 9. Entry function with arguments and a top-level material

```slx
// mxslc: -f main -a 0.25 "textures/wood.png" true
inline void main(float roughness, filename tex, bool flip_u)
{
    vec2 uv = texcoord();
    vec2 st = if (flip_u) { vec2{1.0 - uv.x, uv.y} } else { uv };
    color3 col = image(tex, texcoord = st);
    surfacematerial(standard_surface(base_color = col, specular_roughness = roughness));
}
```

From Python, `func_args` can include MaterialX values:
`mxslc.CompileOptions(func_name="main", func_args=[0.25, "textures/wood.png", True])`.

## 10. Shader variants with macros

```slx
// mxslc: -g tint_strength 0.5
global float tint_strength;
#ifdef OPEN_PBR
surfaceshader s = open_pbr_surface(base_color = mix(randomcolor(), color3{1, 0, 0}, tint_strength));
#else
surfaceshader s = standard_surface(base_color = mix(randomcolor(), color3{1, 0, 0}, tint_strength));
#endif
material m = surfacematerial(s);
```

Select the variant with `CompileOptions(macros=["OPEN_PBR"], globals={"tint_strength": 0.5})`, or with
`#define OPEN_PBR` at the top. The CLI has no macro flag.

## 11. Porting GLSL / Shadertoy code

Map names with object-like macros or inline wrappers, mark everything you reassign `mutable`, replace
`?:` with if-expressions, and use an `out` parameter for `fragColor`. `mix` keeps the MaterialX argument
order, so wrap it.

```slx
#define dot dotproduct
#define iTime time()

inline T glsl_mix<float, vec2, vec3, vec4>(T a, T b, float t) { return mix(b, a, t); }
inline float glsl_step(float edge, float x) { return float{x >= edge}; }
inline float hash21(vec2 p) { return fract(sin(dot(p, vec2{12.9898, 78.233})) * 43758.5453); }

inline void mainImage(out vec4 fragColor, vec2 fragCoord)
{
    vec2 uv = fragCoord * 8.0;
    vec2 id = floor(uv);
    vec2 gv = fract(uv) - 0.5;
    float h = hash21(id);
    mutable float d = magnitude(gv);
    d = glsl_step(d, 0.3 + 0.1 * sin(iTime + h * 6.283));
    vec3 col = glsl_mix(vec3{0.1, 0.1, 0.2}, vec3{h, 0.5, 1.0 - h}, d);
    fragColor = vec4{col, 1.0};
}

mutable vec4 fragColor;
mainImage(out fragColor, texcoord());
surfacematerial(surface_unlit(emission_color = color3{fragColor}));
```

## 12. A reusable custom-node library

Compile this file on its own to get NodeDefs and NodeGraphs (non-inline functions). Then load the result
in other shaders with `#library "noise_lib.mtlx"`, and make the renderer load `noise_lib.mtlx` as well.

```slx
@nodegroup "procedural"
@doc "Fractal brownian motion over 2D value noise"
@out.doc "noise in roughly [-1, 1]"
float fbm2(vec2 texcoord = vec2{0.0}, float lacunarity = 2.0, float gain = 0.5)
{
    mutable float sum = 0.0;
    mutable float amp = 0.5;
    mutable vec2 p = texcoord;
    for (int i from 0 to 4)                     // compile-time count: 5 octaves
    {
        sum += noise2d(texcoord = p) * amp;
        p *= lacunarity;
        amp *= gain;
    }
    return sum;
}

@nodegroup "procedural"
color3 fbm_tint(vec2 texcoord = vec2{0.0}, color3 low = color3{0.0}, color3 high = color3{1.0})
{
    return mix(high, low, fbm2(texcoord) * 0.5 + 0.5);
}
```

## 13. Switch-expression material selection

```slx
int id = geompropvalue("material_id", 0);
color3 base = switch (id)
{
    color3{0.8, 0.1, 0.1},       // 0
    color3{0.1, 0.8, 0.1},       // 1
    case 2: color3{0.1, 0.1, 0.8},
    case >=5: checkerboard(color1 = color3{1}, color2 = color3{0}),
    default: color3{0.5}         // 3, 4
};
surfacematerial(standard_surface(base_color = base));
```

## 14. Parameterless and nodegraph functions

```slx
float facing => { return dotproduct(normal("world"), viewdirection("world")); }

float rim_width = 0.35;

[[nodegraph]]
color3 rim(float width = rim_width, color3 tint = color3{1.0, 0.6, 0.2})
{
    return tint * smoothstep(1.0 - |facing|, 1.0 - width, 1.0);
}

surfacematerial(standard_surface(base_color = color3{0.05}, emission = 1.0, emission_color = rim()));
```
