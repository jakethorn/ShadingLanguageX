<h1 align="center">Basic Examples</h1>

<p align="center">
  <img src="https://github.com/jakethorn/ShadingLanguageX/blob/main/examples/screenshots/combined.png" />
</p>

## Hello, ShadingLanguageX!

#### hello_slx.mxsl
```c++
surfaceshader surf = standard_surface();
surf.base_color = color3{1, 0, 0};

material mat = surfacematerial(surf);
```
#### python script
```python
import mxslc
mxslc.compile_file_to_file("hello_slx.mxsl")
```
#### hello_slx.mtlx
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <standard_surface name="surf" type="surfaceshader">
    <input name="base_color" type="color3" value="1, 0, 0" />
  </standard_surface>
  <surfacematerial name="mat" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="surf" />
  </surfacematerial>
</materialx>
```

[screenshot here]

## Gold

#### gold.mxsl
```c++
surfaceshader goldsurf = standard_surface();
goldsurf.base_color = color3{1.059, 0.773, 0.307};
goldsurf.specular_color = color3{1.001, 0.985, 0.523};
goldsurf.specular_roughness = 0;
goldsurf.metalness = 1;

material goldmat = surfacematerial(goldsurf);
```
#### python script
```python
import mxslc
mxslc.compile_file_to_file("gold.mxsl")
```
#### gold.mtlx
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <standard_surface name="goldsurf" type="surfaceshader">
    <input name="base_color" type="color3" value="1.059, 0.773, 0.307" />
    <input name="specular_color" type="color3" value="1.001, 0.985, 0.523" />
    <input name="specular_roughness" type="float" value="0" />
    <input name="metalness" type="float" value="1" />
  </standard_surface>
  <surfacematerial name="goldmat" type="material">
    <input name="surfaceshader" type="surfaceshader" nodename="goldsurf" />
  </surfacematerial>
</materialx>
```

[screenshot here]

## Toon

#### toon.mxsl
```c++
vec3 i = viewdirection("world");
vec3 n = normal("world");
float theta = dotproduct(-i, n);
color3 c = if (|theta| > 0.65) {
    {1.0, 0.2, 0.143}
} else {
    {0.154, 1.0, 1.0}
};

surfaceshader s = surface_unlit(emission_color = c);
```
#### python script
```python
import mxslc
mxslc.compile_file_to_file("toon.mxsl")
```
#### toon.mtlx
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <viewdirection name="i" type="vector3">
    <input name="space" type="string" value="world" />
  </viewdirection>
  <normal name="n" type="vector3">
    <input name="space" type="string" value="world" />
  </normal>
  <invert name="var__0" type="vector3">
    <input name="in" type="vector3" nodename="i" />
    <input name="amount" type="float" value="0" />
  </invert>
  <dotproduct name="theta" type="float">
    <input name="in1" type="vector3" nodename="var__0" />
    <input name="in2" type="vector3" nodename="n" />
  </dotproduct>
  <absval name="var__1" type="float">
    <input name="in" type="float" nodename="theta" />
  </absval>
  <ifgreater name="var__2" type="boolean">
    <input name="value1" type="float" nodename="var__1" />
    <input name="value2" type="float" value="0.65" />
  </ifgreater>
  <ifequal name="c" type="color3">
    <input name="value1" type="boolean" nodename="var__2" />
    <input name="value2" type="boolean" value="true" />
    <input name="in1" type="color3" value="1, 0.2, 0.143" />
    <input name="in2" type="color3" value="0.154, 1, 1" />
  </ifequal>
  <surface_unlit name="s" type="surfaceshader">
    <input name="emission_color" type="color3" nodename="c" />
  </surface_unlit>
</materialx>
```

[screenshot here]

## Colored Circles

#### circles.mxsl
```c++
global int N;

mutable color3 c = color3{1};
for (int i from 1 to N)
{
    vec2 p = {randomfloat(i, seed=0), randomfloat(i, seed=1)};
    c = if (distance(p, texcoord()) < 0.1) { randomcolor(i) };
}

surfaceshader s = standard_surface(base_color = c);
material m = surfacematerial(s);
```
#### python script
```python
import mxslc
opts = mxslc.CompileOptions(globals={"N": 5})
mxslc.compile_file_to_file("circles.mxsl", opts)
```
#### circles.mtlx
[screenshot here]

[screenshot here]

## Brick Walls

#### brick_wall.mxsl
```c++
#include "brick_pattern.mxsl"

color3 color1;
color3 color2;

color3 c = get_brick_pattern(color1, color2); // from brick_pattern.mxsl
surfaceshader s = standard_surface(base_color=c);
material m = surfacematerial(s);
```
#### python script
```python
import mxslc
import MaterialX as mx

colors = [
    [mx.Color3(0.52, 0.21, 0.21), mx.Color3(0.87, 0.21, 0.22)],
    [mx.Color3(0.90, 0.90, 0.90), mx.Color3(0.70, 0.70, 0.70)],
    [mx.Color3(0.11, 0.12, 0.15), mx.Color3(0.21, 0.22, 0.25)],
    [mx.Color3(0.82, 0.71, 0.55), mx.Color3(0.72, 0.61, 0.45)],
]

for i, color in enumerate(colors):
    opts = mxslc.CompileOptions(
        output_file=f"brick_wall{i}.mtlx",
        globals={"color1": color[0], "color2": color[1]}
    )
    mxslc.compile_file_to_file("brick_wall.mxsl", opts)
```

[screenshot here]

## Interior Mapping

#### interior_mapping.mxsl
```c++
#ifdef USE_RAIN
#include "rain_uvs.mxsl"
#endif

inline vec3 raycast(vec3 origin, vec3 view_dir)
{
    vec3 dn = (-origin - 1) / view_dir;
    vec3 dp = (-origin + 1) / view_dir;
    vec3 d_min = max(dn, dp);
    return origin + min(d_min.x, d_min.y, d_min.z) * view_dir;
}

#ifdef USE_RAIN
vec2 uv = 1 - get_rain_uvs(); // from rain_uvs.mxsl
#else
vec2 uv = 1 - texcoord();
#endif

vec3 origin = vec3{uv * 2 - 1, 1};
vec3 view_dir = viewdirection("object");
vec3 room_pos = raycast(origin, view_dir);

color3 walls =
    if      (room_pos.x >  0.99) { {0, 0, 1} }
    else if (room_pos.x < -0.99) { {1, 0, 0} }
    else if (room_pos.y >  0.99) { {0, 1, 0} }
    else if (room_pos.y < -0.99) { {1, 0, 1} }
    else if (room_pos.z < -0.99) { {1, 1, 0} }
    else                         { {0, 0, 0} };

vec3 light_pos = {
    sin(time() * 0.667) * 0.75,
    cos(time() * 0.5) * 0.75,
    0
};

float light = (1.2 - distance(light_pos, room_pos) / 1.732) ^ 4;
float ao = 0.1 - magnitude(room_pos) / 17.32;

surfaceshader surface = open_pbr_surface();
surface.base_color = walls * max(light, ao);
surface.specular_roughness = 1;

material parallax_mat = surfacematerial(surface);
```
#### python script
```python
import mxslc

opts = mxslc.CompileOptions(output_file="interior_mapping.mtlx", macros=[])
mxslc.compile_file_to_file("interior_mapping.mxsl", opts)

opts = mxslc.CompileOptions(output_file="interior_mapping_with_rain.mtlx", macros=["USE_RAIN"])
mxslc.compile_file_to_file("interior_mapping.mxsl", opts)
```

[screenshot here]

## Brownian Mountains

#### mountain.mxsl
```c++
// uvs are pixels position in world space
const vec2 uv = position().xy;

// brownian noise algorithm
mutable float scale = 2;
mutable float height = 0;

for (int i from 0 to 3)
{
    float scale2 = scale * scale;
    height += noise2d(texcoord = uv / scale2) * scale2;
    scale *= 2;
}

// construct material
surfacematerial(
    standard_surface(
        base_color = color3{0.820, 0.796, 0.761},
        metalness = 0.0,
        specular_roughness = 1.0
    ),
    null,
    displacement(height)
);

```
#### python script
```python
import mxslc
mxslc.compile_file_to_file("mountain.mxsl")
```
#### mountain.mtlx
[screenshot here]

[screenshot here]
