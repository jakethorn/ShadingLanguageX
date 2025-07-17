# Version 0.5.2-beta
## Added
* __Attributes__  
Custom attributes can be added to nodes:
```
@doc "an image of a butterfly"
@file.colorspace "srgb_texture"
color3 c = image("butterfly1.png");
```
```xml
<image name="c" type="color3" doc="an image of a butterfly">
  <input name="file" type="filename" colorspace="srgb_texture" value="butterfly1.png" />
</image>
```
Or NodeDefs:
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
# Version 0.5.1-beta
## Added
* __Auto Keyword__
```
auto pi = 3.14;  
auto red = color3(1.0, 0.0, 0.0);
auto uv = texcoord<vec2>();

auto add_one(float x)
{
    return x + 1.0;
}

auto randomvector<vec2, vec3, vec4>()
{
    float r = randomfloat();
    return T(r);
}
```
# Version 0.5-beta
## Changed
* __Functions__  
Functions now compile to a NodeDef/NodeGraph pair. Previously they were simply "inlined" into the main MaterialX document whenever the function was called.
Functions otherwise operate exactly the same, e.g., variables from outer scopes can be accessed and updated inside of functions.
Functions can also still be declared inside other functions.
```
float add_one(float f) { return f + 1.0; }

float x = add_one(1.0);
float y = add_one(x);
```
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_add_one" node="add_one">
    <output name="out" type="float" default="0.0" />
    <input name="f" type="float" value="0" />
  </nodedef>
  <nodegraph name="NG_add_one" nodedef="ND_add_one">
    <add name="node3" type="float">
      <input name="in1" type="float" interfacename="f" />
      <input name="in2" type="float" value="1" />
    </add>
    <output name="out" type="float" nodename="node3" />
  </nodegraph>
  <add_one name="x" type="float">
    <input name="f" type="float" value="1" />
  </add_one>
  <add_one name="y" type="float">
    <input name="f" type="float" nodename="x" />
  </add_one>
</materialx>
```
Example 2 - Accessing variables from an outer scope.
```
int seed = 0;

// return a different random float each time.
float my_rand()
{
    seed += 1;
    return randomfloat(seed=seed);
}

float x = my_rand();
float y = my_rand();
color3 c = color3(x, y, 0.0);
```
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND_my_rand" node="my_rand">
    <output name="out" type="float" default="0.0" />
    <input name="seed" type="integer" value="0" />
    <output name="seed2" type="integer" default="0" />
  </nodedef>
  <nodegraph name="NG_my_rand" nodedef="ND_my_rand">
    <add name="node4" type="integer">
      <input name="in1" type="integer" interfacename="seed" />
      <input name="in2" type="integer" value="1" />
    </add>
    <output name="seed2" type="integer" nodename="node4" />
    <randomfloat name="node5" type="float">
      <input name="seed" type="integer" nodename="node4" />
    </randomfloat>
    <output name="out" type="float" nodename="node5" />
  </nodegraph>
  <my_rand name="node1" type="multioutput">
    <input name="seed" type="integer" value="0" />
    <output name="out" type="float" />
    <output name="seed2" type="integer" />
  </my_rand>
  <my_rand name="node2" type="multioutput">
    <input name="seed" type="integer" output="seed2" nodename="node1" />
    <output name="out" type="float" />
    <output name="seed2" type="integer" />
  </my_rand>
  <combine3 name="c" type="color3">
    <input name="in1" type="float" output="out" nodename="node1" />
    <input name="in2" type="float" output="out" nodename="node2" />
    <input name="in3" type="float" value="0" />
  </combine3>
</materialx>
```
* __Loops__  
Similar to functions, loops also now compile to a NodeDef/NodeGraph pair.
```
float x = 0.0;

for (float i = 0.0:2.0)
{
    x += randomfloat(i);
}

color3 c = color3(x);
```
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <nodedef name="ND___loop__1" node="__loop__1">
    <input name="i" type="float" value="0" />
    <input name="x" type="float" value="0" />
    <output name="x2" type="float" default="0.0" />
  </nodedef>
  <nodegraph name="NG___loop__1" nodedef="ND___loop__1">
    <randomfloat name="node3" type="float">
      <input name="in" type="float" interfacename="i" />
    </randomfloat>
    <add name="node5" type="float">
      <input name="in1" type="float" interfacename="x" />
      <input name="in2" type="float" nodename="node3" />
    </add>
    <output name="x2" type="float" nodename="node5" />
  </nodegraph>
  <__loop__1 name="x2" type="float">
    <input name="i" type="float" value="0" />
    <input name="x" type="float" value="0" />
  </__loop__1>
  <__loop__1 name="x3" type="float">
    <input name="i" type="float" value="1" />
    <input name="x" type="float" nodename="x2" />
  </__loop__1>
  <__loop__1 name="x4" type="float">
    <input name="i" type="float" value="2" />
    <input name="x" type="float" nodename="x3" />
  </__loop__1>
  <convert name="c" type="color3">
    <input name="in" type="float" nodename="x4" />
  </convert>
</materialx>
```
## Added
* __material Data Type__  
Previously, users only needed to define a `surfaceshader` type and SLX would take this and implicitly create a `surfacematerial` node and pass in the `surfaceshader` variable.
This removed some verbosity from SLX, but tunneled users into only this specific workflow. 
Starting from this new update, users now have to create their own `material` nodes, for example by calling the `surfacematerial(...)` function and pass in their shaders manually.
```
material main(float tiling)
{
    vec2 scaled_uv = texcoord() * tiling;
    float seed = floor(scaled_uv.x) + floor(scaled_uv.y) * tiling;
    color3 c = randomcolor(seed);
    
    return surfacematerial(
        standard_surface(base_color=c)
    );
}
```
In addition, because functions now compile to NodeDef/NodeGraphs instead of being inlined in the main document, `material` variables need to be returned from the function to be accessible from the main document.
While these changes mean the SLX language will become somewhat more verbose when creating `shader`/`material` variables, 
my hope is that it allows users to use more complex workflows that are not limited to just the `standard_surface` and `surfacematerial` nodes. 
Another benefit is to reduce the confusion from SLX performing implicit operations in the background. It's easy for me to know what SLX is doing in the background because I wrote the compiler,
but developers just started to use the language might be surprised by the sudden `surfacematerial` node in their MaterialX shader.  
_"One of the things I like about C++ is that I can predict the machine code it will generate." - Bjarne Stroustrup_

* __Standard Library__  
The SLX standard function library has been expanded to provide access to every NodeDef defined in the `libraries` directory of the MaterialX project, including `stdlib`, `pbrlib`, `nprlib`, `bxdf`, etc...
```
BSDF gem = LamaDielectric();
BSDF copper = LamaConductor();
float mask = image("jewelry_mask.png");
BSDF jewelry = LamaMix(gem, copper, mask);
LamaSurface(jewelry, copper);
```
```xml
<?xml version="1.0"?>
<materialx version="1.39">
  <LamaDielectric name="gem" type="BSDF" />
  <LamaConductor name="copper" type="BSDF" />
  <image name="mask" type="float">
    <input name="file" type="filename" value="jewelry_mask.png" />
  </image>
  <LamaMix name="jewelry" type="BSDF">
    <input name="material1" type="BSDF" nodename="gem" />
    <input name="material2" type="BSDF" nodename="copper" />
    <input name="mix" type="float" nodename="mask" />
  </LamaMix>
  <LamaSurface name="node1" type="material">
    <input name="materialFront" type="BSDF" nodename="jewelry" />
    <input name="materialBack" type="BSDF" nodename="copper" />
  </LamaSurface>
</materialx>
```
* __More Data Types__  
To support the new standard function library, SLX now additionally supports Matrices (`matrix33` and `matrix44`), `volumeshader`, `lightshader`, `material` (as mentioned above), `BSDF` (as seen in the above example), `EDF` and `VDF`.

# Version 0.4.1-beta
## Added
Finished implementing standard library functions to match the MaterialX Standard Nodes.

# Version 0.4-beta
## Added
* __Function Overloading__  
Functions can now be overloaded, allowing them to have the same names as long as their signature is unique. In __ShadingLanguageX__, a functions return type is a part of its signature.
```
float length(vec2 v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

float length(vec3 v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec2 uv2 = geompropvalue("uv2");
vec3 n = geompropvalue("Nworld");

float a = length(vv2);
float b = length(n);
```
```
float random()
{
    return randomfloat();
}

vec2 random()
{
    return vec2(randomfloat(), randomfloat());
}

float f = rand();
vec2 v2 = rand();
```
* __Function Templates__  
Templates allow functions with different data types but the same logic to be implemented in a single function.
```
T frac<float, vec2, vec3, vec4, color3, color4>(T value)
{
    return value % 1.0;
}

float a = 0.7;
vec2 v2 = vec2(1.5, 8.1);
color3 c3 = color3(3.4, 0.1, 9.9);

a = frac(a);
v2 = frac(v2);
c3 = frac(c3);
```
In the previous example, the template type `T` was inferred from the functions usage, but it can be specified if the usage is ambiguous or for code clarity.
```
a = frac<float>(a);
v2 = frac<vec2>(v2);
c3 = frac<color3>(c3);
```
The template type `T` can be used wherever a standard data type could have been used inside the function.
```
T image_mult<vec3, color3>(filename img_path, T mult = T(1.0))
{
    T img = image<T>(img_path);
    return img * mult;
}

color3 brick = image_mult("brick.png", color3(0.8, 0.1, 0.1));
vec3 normal = image_mult("brick_normals.png");
```
* __Default Arguments__  
Function parameters can now have default arguments.
```
float incr(float value, float amount=1.0)
{
    return value + amount;
}

float a = 1.0;
a = incr(a);
a = incr(a, 2.0);
```
* __Null Value__  
Any variable can now be assigned the `null` value. When that variable is compiled and its value assigned to the input of a MaterialX node, the input will not be set. This variable can be used anywhere an expression can be evaluated, but is specifically useful to provide default arguments to functions that do not create inputs in the underlying MaterialX nodes, letting MaterialX define the default value instead of the developer.
```
color3 image(filename file, vec2 texcoord = null)
{
    return {"image", color3: file=file, texcoord=texcoord);
}

color3 c = image("butterfly1.png");
```
* __Named Arguments__  
When calling a function, arguments can be named to target specific parameters.
```
color3 image(filename file, color3 default = null, vec2 texcoord = null)
{
    return {"image", color3: file=file, default=default, texcoord=texcoord);
}

color3 c = image("butterfly1.png", texcoord=geompropvalue("uv2"));
```
# Version 0.3-beta
## Added
* __Expression Statements__  
Allow function and standard library calls to be invoked without assigning the returned value. 
```
vec2 uv = texcoord();
displacement(tangent("object"), uv.x * uv.y);
```
* __Preprocessor Directives__  
C-style preprocessor directives that can control the compilation of the shader.
```
#define USE_SRGB
#define GAMMA 2.2
color3 c = image("albedo.png");
#ifdef USE_SRGB
c ^= GAMMA;
#endif
```
* __Ternary Relational Expression__  
Concise expression for checking if an expression falls within a lower and upper limit.
```
bool is_normalized = -1.0 < x < 1.0;
```
* __Improved Main Function__  
Main function name and arguments can now optionally be specified when calling `mxslc.compile_file(...)`.
```
mxslc.compile_file("my_shader.mxsl", main_function="my_function", main_args=[42, 10.0, Path("butterfly1.png")])
```
```
void my_function(int seed, float n, filename img_path)
{
    float y = 0.0;
    for (float i = 0.0 : n)
    {
        y += randomfloat(i, seed=seed);
    }

    vec2 uv = vec2(texcoord().x, y / n);
    surfaceshader surface = standard_surface();
    surface.base_color = image(img_path, texcoord=uv);
}
```
* __Additional Compile Arguments__  
Added several compile arguments to `mxslc.compile_file(...)` and mxslc executable.
```
positional arguments:
  mxsl_path                        Input path to mxsl file or containing folder

options:
  -h, --help                       show this help message and exit
  -o, --output-path OUTPUT_PATH    Output path of generated mtlx file or containing folder
  -m, --main-func MAIN_FUNC        Name of main entry function into the program
  -a, --main-args MAIN_ARGS        Arguments to be passed to the main function
  -i, --include-dirs INCLUDE_DIRS  Additional directories to search when including files
  -d, --define MACROS              Additional macro definitions
```
# Version 0.2-beta
## Added 
* __Node Constructor__  
Manually define a MaterialX nodes category, type and inputs.  
```
vec3 n = image("normals.png");
vec3 v = {"normalmap", vec3: in=n, space="tangent", scale=0.1};
```
###
* __Include Directive__  
Specify other files to be included in compilation.
```
#include "some_functions.mxsl"
```
###
* __Indexer Expression__  
Extract vector and color channels using square brackets.
```
vec3 n = normal();
float x = n[0];
```
###
* __Displacement Shader Support__
```
float height = image("heightmap.png");
displacementshader ds = displacement(tangent(), height);
```
# Version 0.1-beta
## Supported Features
* __Variable Declaration__
* __Variable Assignment__
* __Compound Assignment__
* __Function Declaration__
* __For Loop__
* __Binary Operators__
* __Unary Operators__
* __Grouped Expression__
* __Constructors__
* __Explicit Conversions__
* __Standard Library Calls__
* __User-defined Function Calls__
* __Swizzle Operator__
* __If Expression__
* __Surface Shader Output__
