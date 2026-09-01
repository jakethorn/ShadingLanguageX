
  
<h1 align="center">ShadingLanguageX</h1>

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://github.com/jakethorn/ShadingLanguageX/blob/main/LICENSE)
![version](https://img.shields.io/badge/version-0.2.1_beta-blue)
[![Automated Tests](https://github.com/jakethorn/ShadingLanguageX/actions/workflows/automated-tests.yml/badge.svg)](https://github.com/jakethorn/ShadingLanguageX/actions/workflows/automated-tests.yml)

__ShadingLanguageX__ is a high level programming language that can be used to create complex [MaterialX](https://materialx.org/) shaders. Click [here](https://youtu.be/n0-5Tx9cS58?si=tRpTGt7ZWPGW0eh0) to see the ASWF talk from SIGGRAPH 2025.


  
![](examples/screenshots/readme_example.png)


# How It Works
![](examples/screenshots/howitworks.jpg)  

__ShadingLanguageX__ source files are compiled to MaterialX (.mtlx) files using the mxslc compiler. Internally, the source file is tokenized and parsed 
into a tree of statements and expressions which in turn map to one or more MaterialX nodes. These nodes are then written to the MaterialX output file 
as shown in the diagram above.  
For example, the `+` operator (e.g., `float x = 1.0 + 1.0;`) intuitively compiles to the `add` node, and the same for all other mathematical operators. 
`if` expressions compile to either of the `ifgreater`, `ifgreatereq` or `ifequal` nodes depending on the condition. `switch` expressions compile to 
the `switch` node. The swizzle operator (e.g., `some_vector.xy`) compiles to `extract` and `combine` nodes. Most MaterialX nodes are represented by a 
standard library function that is built into the language, such as `color3 c = image("albedo.png");`, which compiles to the `image` node, 
or `vec3 p = position()`, which compiles to the `position` node. Additionally, 
declaring a variable (e.g., `vec3 up = vec3{0.0, 1.0, 0.0};`) compiles to a `constant` node (or a `combine` node depending on the inputs to the expression).  


# Why Use ShadingLanguageX?

Currently, MaterialX shaders can be made either using the official MaterialX API, or using a graph editor software. __ShadingLanguageX__ offers an additional way to create MaterialX shaders that provides several benefits over existing methods.
* __Start and Iterate Quickly__ The MaterialX API can be quite verbose to use when writing shaders because it tries to remain agonistic to the current nodes provided by the specification, and needs to provide control over every aspect of MaterialX. Developers can write their own wrappers around the API, but this takes time and knowledge about MaterialX and either C++ or Python. __ShadingLanguageX__ provides less functionality than the MaterialX API, but in return provides a language with a simple syntax, that was developed specifically for building MaterialX shaders. This allows developers to get started and iterate on ideas quickly. There is no setup code to write, just the shaders and a call to the compiler.
* __Manage Compexity and Reuse Code__ Similarly, graph editors can become difficult to use when developing shaders with a large number of nodes and often have limited function reusability features between shaders. __ShadingLanguageX__ provides for loops, user-defined functions and `#include` directives that make it easier to create shaders with thousands of nodes and reuse code between projects.
* __Shader Readability__ Another benefit of __ShadingLanguageX__ is that it is very readable. As mentioned previously, the MaterialX API can be quite verbose, obscuring the logic of the shader and staring at a network of nodes is not much easier.  __ShadingLanguageX__ has a concise syntax targetted specifically for MaterialX shaders which results in code with logic that is more easily understandable.


# Showcases
|               Procedural Rain               |               Interior Mapping                |        Shader Art (by Kishimisu)        |
|:-------------------------------------------:|:---------------------------------------------:|:---------------------------------------:|
|     ![](examples/screenshots/rain.png)      | ![](examples/screenshots/interiormapping.png) | ![](examples/screenshots/shaderart.png) |
|            __Procedural Waves__             |           __Displacement Mountain__           |                __Toon__                 |
|     ![](examples/screenshots/waves.png)     |    ![](examples/screenshots/mountain.png)     |   ![](examples/screenshots/toon.png)    |
|             __Disintegration__              |              __Colored Squares__              |                __Gold__                 |
| ![](examples/screenshots/disintergrate.png) |     ![](examples/screenshots/squares.png)     |   ![](examples/screenshots/gold.png)    |


# Getting Started

## Installation
__ShadingLanguageX__ source files are compiled to MaterialX (.mtlx) files using its open source compiler (`mxslc`). The 
compiler is written in C++, but provides Python and JavaScript bindings. The python bindings can be downloaded from PyPI 
using `pip install mxslcxx`.

`mxslc` is supported on Windows, MacOS and Linux and the Python bindings have been tested on Python 3.9+.

```python
import mxslc
mxslc.compile_file("my_shader.mxsl")
```
Alternatively, you can download the compiler executable from the most recent release and call it from the command line.
```
> ./mxslc.exe my_shader.mxsl
```
Both examples will output a `my_shader.mtlx` file which can then be used as you would any other MaterialX file. 
Both methods have the same input signature, a mandatory path to a __ShadingLanguageX__ source file and then several 
optional arguments, such as setting the output files directory and name (see the [User Guide]() for more information).

## Language Specification
For information regarding __ShadingLanguageX__ syntax, see the language specification [document](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/LanguageSpecification.md). Examples of __ShadingLanguageX__ shaders in addition to the one below can be found in the examples [directory](https://github.com/jakethorn/ShadingLanguageX/tree/main/examples).

## Example
```
// squares.mxsl

global float tiling;

vec2 scaled_uv = texcoord() * tiling;
float seed = floor(scaled_uv.x) + floor(scaled_uv.y) * tiling;
color3 c = randomcolor(seed);

material m = surfacematerial(
    standard_surface(base_color=c)
);
```
Compile using python:
```python
import mxslc
mxslc.compile_file_to_file("squares.mxsl", globals={"tiling": 10.0})
```
or executable:
```bash
> ./mxslc.exe squares.mxsl -g tiling 10.0
```
![](https://github.com/jakethorn/ShadingLanguageX/blob/main/examples/screenshots/squares.png)

# Contributing
Please try out __ShadingLanguageX__ and start a discussion about a feature you'd like to see or an issue if you find a bug, or feel free to contribute directly to the project by opening a pull request!
