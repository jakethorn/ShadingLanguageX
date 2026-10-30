# ShadingLanguageX for Houdini

A Houdini plugin for writing Solaris materials in [ShadingLanguageX](https://github.com/jakethorn/ShadingLanguageX).

- **SLX Material LOP**: compiles an `.mxsl` file, or SLX code typed into the node, into MaterialX materials on the
  USD stage. It recompiles when you save the file, can bind its materials to geometry, and exposes every compile
  option: MaterialX version, entry function, globals and macros.
- **Convert to SLX**: turns a Material Library, a Karma or MaterialX Material Builder, or a `.mtlx` file into an
  SLX Material node holding the equivalent SLX code.
- **SLX shelf**: new SLX Material, Recompile All, Convert to SLX, Import MaterialX, and a link to the docs.

It needs Houdini 19.5 or later on Windows, Linux or Apple Silicon macOS. So far it has only been tested with
Houdini 22.0 on Windows.

## Installing

1. Get the plugin, either by cloning this repository or from a release.
2. Download the compiler for every Houdini version and platform. This needs any Python 3 and an internet connection:

   ```bash
   python tools/houdini/vendor_mxslc.py
   ```

   This puts the [mxslcxx](https://pypi.org/project/mxslcxx/) wheels into `slx/vendor/`. If you already have
   `mxslcxx` installed into Houdini's Python, the plugin uses that whenever there's no vendored copy.
3. Copy `ShadingLanguageX.json` into the `packages` folder of your Houdini preferences, e.g.
   `~/Documents/houdini22.0/packages` on Windows. Then point it at this folder's `slx` directory. Either set an
   `SLX_HOUDINI` environment variable to that path, or replace the `C:/path/to/...` default in the file.
4. Start Houdini. Add the **SLX** shelf from the shelf's **+** menu.

## The SLX Material LOP

Find it in the LOP Tab menu under **Materials > SLX Material**. Each `material` variable in the SLX source becomes a
material primitive, named after the variable, under the **Material Path Prefix** (`/materials/` by default).

```slx
surfaceshader surface = standard_surface(base_color = color3{0.8, 0.2, 0.1});
material red = surfacematerial(surface);    // -> /materials/red
```

| Tab | What it does |
|---|---|
| Source | Choose **File** or **Inline Code**. **Compile** rereads the source. With **Recompile When Files Change** on, saving the file, or any file it `#include`s, recompiles it. **Open in Editor** uses `$SLX_EDITOR`, `$VISUAL` or `$EDITOR`, then VS Code, then your OS default. If the last compile failed, VS Code opens at the error line. |
| Materials | **Assignments** bind compiled materials to geometry from the node's input. |
| Compile Options | MaterialX version (by default it matches Houdini's), entry function and arguments, globals, macros, graph reduction, and **Expand Function Nodes**. |
| Output | **Cache Directory** for the compiled `.mtlx` files, and buttons to view or export the compiled MaterialX. |

Things to know:

- The materials reference compiled `.mtlx` files in the **Cache Directory** (`$HIP/slx_cache` by default). Renders,
  including render farm jobs, must be able to read that directory. Each change writes a new file, named after its
  contents, so USD never shows a stale material. Old files aren't deleted, because USD files you exported earlier
  may still use them.
- Karma and the viewport only render MaterialX's standard nodes. **Expand Function Nodes** (on by default) replaces
  each node made from a non-inline SLX function with the nodes inside it. **View** and **Export MaterialX** show the
  document before expansion, with your functions as NodeDefs.
- Compile errors appear on the node. Houdini wraps them in "Invalid source ...", because the compile runs inside
  the asset.

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
