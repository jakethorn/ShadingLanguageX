## Visual Studio Code Extension Builder

This folder contains scripts to build a extension for VSCode as well as a pre-built
extension which is built against MaterialX 1.39.5.

Key features supported include:

- Interactive syntax highlighting for ShaderLanguageX files (`.MXSL` extension)
- Extension commands for bi-directional validation and/or conversion between MaterialX (`.MTLX`) and ShaderLanguageX (`.MXSL`). Validation emits diagnostic information for any conversion issues.

### Command Script

The main command script is `gen_extension.py` which can be used to generate and or install
the extension.

#### Requirements

- Installed Python module for MaterialX to allow discovery of MaterialX definition names. If not installed, syntax highlighting will not highlight these names, but will still work fully.

The command has the following syntax:

```
gen_extension.py [-h] [-g] [-i] [-v] [--mtlx-stdlib MTLX_STDLIB] [-c] [--js-build-dir JS_BUILD_DIR] [--build-js] [--materialx-root MATERIALX_ROOT]
                        [--emsdk-location EMSDK_LOCATION] [-p]

Generate and/or install a VS Code extension for MXSL syntax highlighting.

options:
  -h, --help            show this help message and exit
  -g, --generate        Generate extension files under tools/vscode_extension/mxsl-vscode/
  -i, --install         Package as .vsix and install via code --install-extension
  -v, --vsix            Package the extension as mxsl-language-<version>.vsix
  --mtlx-stdlib MTLX_STDLIB
                        Path to the MaterialX libraries/ directory (containing stdlib/, bxdf/, etc.). Defaults to the installed MaterialX package's built-in libraries.
  -c, --converter       Bundle the MXSL<->MTLX WebAssembly converter into the extension: copies JsMxslc.{js,wasm,data} into media/lib and adds a converter webview command.
  --js-build-dir JS_BUILD_DIR
                        Path to the JS build 'bin' directory containing JsMxslc.{js,wasm,data}. Defaults to mxslc++/javascript/build/mxslc/javascript/bin.
  --build-js            Build the JS/WASM module with build_javascript.sh even if the artifacts already exist. Auto-builds whenever artifacts are missing.
  --materialx-root MATERIALX_ROOT
                        Path to a MaterialX source tree (used when building the JS module). Defaults to the MATERIALX_ROOT environment variable.
  --emsdk-location EMSDK_LOCATION
                        Path to the Emscripten SDK (used when building the JS module). Defaults to <repo>/../emsdk.
```

### Installing

The pre-built extension can be installed by using the `-i/--install` argument only.
The extension file (`.vsix`) can be also be manually installed. 

### Building

To build locally the recommended options are to (re)generate the extension (`.vsix` file) and install it. (`-g/--generate` and `-i/--install` respectively). 

For support of MaterialX definition names for syntax highlighting, install the appropriate version of 
the MaterialX Python module. e.g. Use `pip install MaterialX` from PyPi.

For conversion and validation support the `-c/--converter` option should be specified.
It will test for the existence of the required Javascript module in the Javascript build area.
An alternative user location can be specified using the `--js-build-dir` argument.

If the module is not found a local build can be invoked, by providing both the appropriate
MaterialX source location (`--materialx-root`), and EMSDK location (` --emsdk-location`) See the [README.md](../../mxslc++/javascript/README.md) in the `msxlc++/javascript` folder for more details.

Note that the libraries can just be built using the `--build-js` argument.

### Example Usage

- Install pre-built extension (`.vsix`)
```
python gen_extension.py --install
```

- Generate and install 
```
python gen_extension.py --generate --install
```

- Also include the convertor, build
```
python gen_extension.py --generate --install --converter --materialx-root mxslc++/MaterialX-1.39.5 --emsdk-location ../emsd
```

- Also force rebuild of JS modules.
```
python gen_extension.py --generate --install --converter --build-js --materialx-root mxslc++/MaterialX-1.39.5 --emsdk-location ../emsd
```






