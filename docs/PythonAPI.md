# mxslc Python API

mxslc consists of a large Python codebase, but only a small subset is needed to use the compiler. The exposed functions and classes are as follows:  
* Functions  
  - `compile_file`
* Classes  
  - `Macro`  
  - `InteractiveCompiler`
  - `ShaderInterface`
* Types  
  - `Value`  

## `compile_file`
`compile_file` provides the same functionality as the mxslc executable. It takes in a path to a ShadingLanguageX source file and generates a MaterialX (.mtlx) file. The function signature is as follows:
```
def compile_file(mxsl_path: str | Path,
                 mtlx_path: str | Path | None = None,
                 *,
                 main_func: str | None = None,
                 main_args: Sequence[mxslc.Value] | None = None,
                 add_include_dirs: Sequence[Path] | None = None,
                 add_macros: Sequence[str | mxslc.Macro] | None = None
                 validate: bool = False) -> None
```

### Example

```
material color_shader(float r, float g, float b)
{
    return surfacematerial(
        standard_surface(base_color=color3(r, g, b))
    );
}
```
```python
import mxslc
mxslc.compile_file("example.mxsl", "cyan.mtlx", main_func="color_shader", main_args=[0.0, 1.0, 1.0])
```  

![]()
  
```
material main()
{
    color3 c = image("albedo.png");
#ifdef SRGB
    c ^= GAMMA;
#endif
    return surfacematerial(
        standard_surface(base_color=c)
    );
}
```
```python
import mxslc
srgb = mxslc.Macro("SRGB")
gamma = mxslc.Macro("GAMMA", 2.2)
mxslc.compile_file("example.mxsl", add_macros=[srgb, gamma])
```

## `Value`

`Value` is a type definition with the following signature:
```
type Value = mx.Node | bool | int | float | mx.Vector2 | mx.Vector3 | mx.Vector4 | mx.Color3 | mx.Color4 | str | Path
```
`mx` is the official MaterialX Python package which can be downloaded via pip.

## `Macro`

`Macro` can be used to define macros that then get passed to the `compile_file` function.
```
class Macro(identifier: str, value: str | None = None)
```

## `InteractiveCompiler`

The `InteractiveCompiler` is still under development; crashes and incorrect behaviour can be expected when using it. That being said, the functionality is extremely powerful. It allows python to call functions and access variables from your ShadingLanguageX shader during compilation, giving it control over the execution flow of statements. This is useful if more complex logic is needed during compilation that is not possible to achieve using only ShadingLanguageX, such as checking for missing textures files, or updating values or logic based on a configuration file.
```
class InteractiveCompiler(add_include_dirs: Sequence[Path] = None)
    # properties
    document: mx.Document
    xml: str

    # functions
    def get_shader_interface() -> ShaderInterface
    def include(mxsl_path: str | Path) -> None
    def eval(code_snippet: str) -> None
    def save(mtlx_filepath: Path, mkdir=False) -> None
    def clear() -> None
```

### `document`

Returns the MaterialX document that is being written to by mxslc. `mx.Document` is defined in the official MaterialX Python API package.

### `xml`

Returns the current xml string of the MaterialX document.

### `get_shader_interface()`

Returns the shader interface. This is what lets you interact with the shader. See [`ShaderInterface`](https://github.com/jakethorn/ShadingLanguageX/blob/main/docs/PythonAPI.md#shaderinterface) section below for more information.

### `include`

Loads a ShadingLanguageX source file and includes it in the current compilation process. It is analogous to the `#include` directive.

### `eval`

Works the same as `include`, but directly on a piece of code passed in as a `string` instead of loading a source file.

### `save`

Finishes compilation and saves the MaterialX file to the file specified by `mtlx_filepath`.

### `clear`

Clears the MaterialX document, removing all previously compiled nodes.

## `ShaderInterface`

```
class ShaderInterface()
    # properties
    global variables in the shader

    # functions
    global functions in the shader
```

`ShaderInterface` has dynamic members that reflect the global variables and functions of the shaders included during the compilation. For example:
```
// my_shader.mxsl

float foo = 0.0;
vec3 bar = color3();

void do_something(vec3 v)
{
    // does something...
}
```
```python
import MaterialX as mx
import mxslc
compiler = mxslc.InteractiveCompiler()
compiler.include(Path("my_shader.mxsl"))
shader = compiler.get_shader_interface()
```
`shader` would now be able to do the following:
```python
shader.foo = 1.0
shader.do_something(shader.bar)
shader.do_something(mx.Vector3(1.0, 0.0, 0.0))
```
which would result in the `foo` variable being set to `1.0` and the invocations of the `do_something` function.  
Additionally, `ShaderInterface` supports in the `in` operator in Python, allowing users to check the presence of variables or functions in the source file.
```python
has_foo = "foo" in shader
```

### Example

```
color3 logic_1(filename texture_path)
{
    // some logic here
}

color3 logic_2(color3 c)
{
    // some other logic here
}

material main(color3 c)
{
    return surfacematerial(
        standard_surface(base_color=c)
    );
}
```
```python
from pathlib import Path
import MaterialX as mx
import mxslc

compiler = mxslc.InteractiveCompiler()
compiler.include(Path("./my_shader.mxsl"))

shader = compiler.get_shader_interface()
texture_path = Path("../textures/my_texture.png")
if texture_path.exists():
    c = shader.logic_1(texture_path)
else:
    c = shader.logic_2(mx.Color3(1, 0, 0))
shader.main(c)

compiler.save(Path("./my_shader.mtlx"))
```
