# MXSL Language Support

Syntax highlighting for the MXSL (MaterialX Shading Language) files in
Visual Studio Code.

## Features
- Syntax highlighting for `.mxsl` files
- Convert the active `.mxsl` file to MaterialX XML (MTLX)
- Convert the active `.mtlx` / `.xml` file to MXSL
- Validate the active file without creating a new one

## Commands

| Command | Description |
| --- | --- |
| `MXSL: Convert between MXSL and MTLX` | Convert the open file and show the result in a new editor |
| `MXSL: Validate current file (MXSL <-> MTLX)` | Run the conversion without creating a new file |

Run these from the Command Palette (`Cmd/Ctrl+Shift+P`) or the editor
context menu on a `.mxsl` / `.mtlx` file.

## Conversion engine
The converter uses the `mxslc` compiler/decompiler compiled to
WebAssembly and bundled with the extension. No network access is required.

## Acknowledgments
This extension bundles the MaterialX standard library (MaterialX,
Copyright (c) Contributors to the MaterialX Project), which is
licensed under the Apache License, Version 2.0. See
<https://www.materialx.org> for details.

## Requirements
- Visual Studio Code 1.85 or newer

## License
Apache-2.0
