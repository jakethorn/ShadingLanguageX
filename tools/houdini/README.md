# ShadingLanguageX for Houdini

A Houdini plugin for writing Solaris materials in [ShadingLanguageX](https://github.com/jakethorn/ShadingLanguageX).

- **SLX Material LOP**: compiles an `.mxsl` file, or SLX code typed into the node, into MaterialX materials on the
  USD stage. Each `global` variable in the source becomes a parameter on the node. It recompiles when you save the
  file, can bind its materials to geometry, loads the ShadingLanguageX examples, and exposes every compile option.
- **Node library**: turns the functions in a folder of SLX files into MaterialX nodes, which Karma and MaterialX
  Material Builders list in their Tab menu.
- **Convert to SLX**: turns a Material Library, a Karma or MaterialX Material Builder, or a `.mtlx` file into an
  SLX Material node holding the equivalent SLX code.
- **SLX shelf**: new SLX Material, Recompile All, Convert to SLX, Import MaterialX, Edit and Rebuild Node Library,
  and a link to the docs.

It needs Houdini 19.5 or later on Windows, Linux or Apple Silicon macOS. So far it has only been tested with
Houdini 22.0 on Windows.

## Installing

1. Download `ShadingLanguageX-Houdini.zip` from a [release](https://github.com/jakethorn/ShadingLanguageX/releases)
   and unzip it. It holds the compiler for every Houdini version and platform, and the examples.
2. Copy `ShadingLanguageX.json` into the `packages` folder of your Houdini preferences, e.g.
   `~/Documents/houdini22.0/packages` on Windows. Then point it at the unzipped `slx` folder. Either replace the
   `C:/path/to/...` default in the file, or set an `SLX_HOUDINI` environment variable to that path. The file also
   puts the [node library](#node-library) on `HOUDINI_PATH`.
3. Start Houdini. Add the **SLX** shelf from the shelf's **+** menu.

To run the plugin from a clone of this repository instead, point the package file at `tools/houdini/slx`, then
download the compiler into `slx/vendor/` with any Python 3:

```bash
python tools/houdini/vendor_mxslc.py
```

`vendor_mxslc.py --wheel` vendors wheels you built yourself. If there's no vendored compiler, the plugin uses an
`mxslcxx` installed into Houdini's Python.

## The SLX Material LOP

Find it in the LOP Tab menu under **Materials > SLX Material**. Each `material` variable in the SLX source becomes a
material primitive, named after the variable, under the **Material Path Prefix** (`/materials/` by default).

```slx
surfaceshader surface = standard_surface(base_color = color3{0.8, 0.2, 0.1});
material red = surfacematerial(surface);    // -> /materials/red
```

| Tab | What it does |
|---|---|
| Source | Choose **File** or **Inline Code**. **Compile** rereads the source. With **Recompile When Files Change** on, saving the file, or any file it `#include`s, recompiles it. **Open in Editor** uses `$SLX_EDITOR`, `$VISUAL` or `$EDITOR`, then VS Code, then your OS default. If the last compile failed, VS Code opens at the error line. **Load Example...** copies an example into `$HIP/slx_examples` and compiles it. |
| Globals | A parameter for each `global` variable in the source, set to its initializer. See below. |
| Materials | **Assignments** bind compiled materials to geometry from the node's input. |
| Compile Options | MaterialX version (by default it matches Houdini's), entry function and arguments, macros, graph reduction, and **Inline SLX Functions**. |
| Output | **Cache Directory** for the compiled `.mtlx` files, and buttons to view or export the compiled MaterialX. |

### Globals

```slx
global float roughness = 0.35;          // a slider, at 0.35
global color3 tint = color3{1, 0.5, 0}; // a colour picker
```

`float`, `int`, `bool`, `string`, `filename`, vector and colour globals each get a parameter. Like any Houdini
parameter, they can be keyframed or driven by expressions, which gives animated materials. Other globals, such as
structs and globals whose initializer isn't known at compile time (e.g. `texcoord().x`), are listed but keep their
initializer.

When you change a global's initializer in the source, its parameter follows, unless you've changed the parameter.
Listing globals needs mxslcxx 0.3.3 or later.

### Things to know

- Relative file paths in the source, such as `image("textures/wood.png")`, are relative to the SLX file, or to
  `$HIP` for inline code.
- The materials reference compiled `.mtlx` files in the **Cache Directory** (`$HIP/slx_cache` by default). Renders,
  including render farm jobs, must be able to read that directory. Each change writes a new file, named after its
  contents, so USD never shows a stale material. Old files aren't deleted, because USD files you exported earlier
  may still use them.
- Karma and the viewport only render MaterialX's standard nodes, not the NodeDefs that SLX functions compile to.
  **Inline SLX Functions** (on by default) compiles every function without a `nodedef`, `[[nodegraph]]`, `inline` or
  `comptime` modifier as `inline`. Turn it off, or mark a function `nodedef`, to get NodeDefs for other renderers.
- Compile errors appear on the node. Houdini wraps them in "Invalid source ...", because the compile runs inside
  the asset.

## Node library

Every non-inline function in the `.mxsl` files of the node library folder becomes a MaterialX node. TDs can write a
node in code and artists use it like any other node in a Karma or MaterialX Material Builder.

```slx
@doc "Repeating ramps from 0 to 1."
float stripes(float x, float count = 8.0)     // the "SLX Stripes" node, with inputs x and count
{
    return fract(x * count);
}
```

**Edit Node Library** on the SLX shelf opens the folder, starting it with an example file. **Rebuild Node Library**
compiles it and makes the nodes, which appear in the Tab menu under **MaterialX > Shading Language X** straight
away.

- Parameter defaults become the node's defaults, and `@doc` attributes its help. `@nodegroup "procedural2d"` files
  a node under another submenu.
- A templated function, such as `T scale_by<float, vec3>(T x)`, becomes one node with a signature per type. If its
  name has an underscore, Houdini labels the signatures oddly, e.g. "byvector3".
- Inline functions aren't nodes, so use them for helpers.

Things to know:

- **Restart Houdini before rendering new or changed nodes in the viewport.** Karma loads node definitions when it
  starts. Renders to disk and on a farm use them straight away.
- The folder is `$SLX_NODE_LIBRARY`, which defaults to `$HOUDINI_USER_PREF_DIR/slx_nodes`. Houdini only puts it on
  `HOUDINI_PATH` if it exists when Houdini starts, so restart once after creating it.
- Every machine that renders with the nodes needs the folder on its `HOUDINI_PATH`. For a render farm or a team,
  set `SLX_NODE_LIBRARY` to a shared folder, and rebuild it with the Houdini version and licence you render with.
- Rebuilding writes `materialx/libraries/slx/*.mtlx` (the node definitions), `karmaNodeGraphs.json` (what Karma
  needs to render them, made with Houdini's `mtlx2karma.py`) and `otls/slx_nodes.hda` (the nodes, made with
  Houdini's `mtlx2hda.py`) into the folder. Other renderers that read your USD files need the `.mtlx` files too.

## Converting materials to SLX

Right-click a Material Library LOP, a material inside one, or a LOP that loads a `.mtlx` file (such as a Reference
LOP), and choose **Convert to SLX Material**. The **Convert to SLX** shelf tool does the same for the selected nodes.
**Import MaterialX** decompiles a `.mtlx` file you choose.

Materials are converted from the USD stage, so this works for Karma Material Builders, which Houdini's own MaterialX
exporter can't save. Shaders inside UsdShade NodeGraph prims, and nodes that aren't standard MaterialX nodes, can't
be converted yet.

## How the node type is installed

The SLX Material asset isn't shipped as a file. Your Houdini builds it the first time it's needed and caches it in
`$HOUDINI_USER_PREF_DIR/slx`. That way the asset always matches your Houdini version and licence. An asset saved
under a non-commercial licence would otherwise switch a commercial session into non-commercial mode.

The asset is built before the first scene file loads, or when an SLX tool first needs it. A hython script that
creates SLX Material nodes without loading a scene first should make sure the asset exists:

```python
import slx_houdini.node_type
slx_houdini.node_type.ensure_node_type()
hou.node("/stage").createNode("slx_material::1.0")
```

## Tests

```bash
python tools/houdini/tests/test_compiler.py
HOUDINI_PATH="/path/to/tools/houdini/slx;&" hython tools/houdini/tests/test_houdini.py
```

Set `HOUDINI_USER_PREF_DIR` to a temporary folder whose name contains `__HVER__` to keep the test's asset build out
of your preferences.

## Packaging

The Publish Wheels workflow builds `ShadingLanguageX-Houdini.zip` from the wheels it builds, and attaches it to the
release. To build it yourself:

```bash
python tools/houdini/package_plugin.py --wheel dist/*.whl    # or --version 0.3.3 for wheels on PyPI
```

The zip only includes the examples that compile with those wheels.
