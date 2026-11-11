## ShadingLanguageX (SLX) for VS Code

This folder holds the VS Code extension for ShadingLanguageX. It provides highlighting, errors as you type,
completion, hovers, parameter hints, Go to Definition, an outline, snippets, a live MaterialX preview, and commands
to compile `.mxsl` files to MaterialX and decompile `.mtlx` files to SLX. See
[slx-vscode/README.md](slx-vscode/README.md) for what it does from a user's point of view.

### Installing

Download `mxslc_vscode_extension.zip` from a [release](https://github.com/jakethorn/ShadingLanguageX/releases), or
the `mxslc-vscode` artifact of a *Build JavaScript (WASM)* workflow run, and unzip it. Then install the `.vsix`:

```
code --install-extension slx-0.4.0.vsix
```

or use *Extensions: Install from VSIX...* in VS Code. If *MXSL Language Support* (the extension's old name) is
installed, uninstall it first.

### Layout

| Path | What it is |
| --- | --- |
| `slx-vscode/` | The extension: `package.json`, TypeScript source in `src/`, tests in `test/` |
| `slx-vscode/src/` | The source. See below. |
| `slx-vscode/syntaxes/slx.tmLanguage.json` | The highlighting grammar. Generated, don't edit it by hand. |
| `slx-vscode/data/` | The keywords, types, MaterialX node signatures, links to the MaterialX specification and a copy of `stdlib.mxsl`, for completion, hovers and parameter hints. Generated and not committed. |
| `slx-vscode/media/lib/` | The WebAssembly compiler. Copied in and not committed. |
| `slx-vscode/skills/` | A copy of the skill for AI agents from [.claude/skills](../../.claude/skills). Copied in and not committed. |
| `grammar/slx.tmLanguage.template.json` | The template the grammar is generated from |
| `gen_extension.py` | Generates the grammar, bundles the compiler, packages and installs the extension |

The source in `slx-vscode/src/`:

| Files | What they do |
| --- | --- |
| `extension.ts` | The entry point: creates everything below |
| `engine.ts`, `engineWorker.ts`, `engineProtocol.ts` | Run the WebAssembly compiler on a worker thread |
| `includes.ts`, `documents.ts` | Find the files a document includes and pass them to the compiler |
| `libraryNodes.ts` | Read the node definitions in `#library` files |
| `diagnostics.ts`, `compileErrors.ts`, `compilerOutput.ts` | Errors as you type, and the compiler's printed output |
| `skill.ts`, `skillFiles.ts` | SLX: Install AI Skill |
| `commands.ts`, `compiledFiles.ts`, `saveDialog.ts`, `preview.ts`, `statusBar.ts` | Commands, the file each SLX file compiles to, the default file of the save dialog, the live preview and the status bar item |
| `macros.ts`, `semanticTokens.ts` | Find macros and highlight their uses, which the grammar can't see |
| `lexer.ts`, `symbols.ts`, `callContext.ts` | Find the declarations in SLX source, and the call at a position. They match shapes and know nothing about types. |
| `languageData.ts`, `signatures.ts` | Read `data/` and format node and function signatures |
| `symbolIndex.ts`, `wordContext.ts` | The declarations a document can use, and what the word at a position is |
| `completion.ts`, `hover.ts`, `signatureHelp.ts`, `navigation.ts`, `outline.ts` | The language features |

The files that don't import `vscode` have unit tests in `test/`.

### Requirements

- Node.js 20 or newer, with npm.
- Python 3.10 or newer, for `gen_extension.py`.
- The WebAssembly build of the compiler (`JsMxslc.js`, `.wasm` and `.data`). See
  [mxslc++/javascript/README.md](../../mxslc++/javascript/README.md). `gen_extension.py --converter` builds it if it
  is missing, given `--materialx-root` and `--emsdk-location`.

### gen_extension.py

```
gen_extension.py [-h] [-g] [-c] [-v] [-i] [-p] [--js-build-dir JS_BUILD_DIR] [--build-js]
                 [--materialx-root MATERIALX_ROOT] [--emsdk-location EMSDK_LOCATION]

  -g, --generate        Regenerate syntaxes/slx.tmLanguage.json and the MaterialX version list in package.json.
  -c, --converter       Copy the WebAssembly compiler (JsMxslc.{js,wasm,data}) into media/lib, building it first if
                        it is missing.
  -v, --vsix            Build the extension and package it as slx-<version>.vsix.
  -i, --install         Package the extension and install it with 'code --install-extension'.
  -p, --publish         Publish to the VS Code Marketplace (needs a VSCE_PAT for the publisher).
  --js-build-dir        Folder holding JsMxslc.{js,wasm,data}. Defaults to mxslc++/javascript/build/mxslc/javascript/bin.
  --build-js            Rebuild the WebAssembly compiler even if it already exists.
  --materialx-root      MaterialX source tree, for building the WebAssembly compiler. Defaults to MATERIALX_ROOT.
  --emsdk-location      Emscripten SDK, for building the WebAssembly compiler. Defaults to <repo>/../emsdk.
```

`--generate` writes the grammar and `data/`, from:

- the keywords and data types in [mxslc++/browser_shared/keywords.js](../../mxslc++/browser_shared/keywords.js),
- the type aliases and functions in [mxslc++/libraries/stdlib.mxsl](../../mxslc++/libraries/stdlib.mxsl),
- the node and type names in the MaterialX libraries in [mxslc++/libraries](../../mxslc++/libraries), for every
  version there. These versions are also the choices for the `slx.materialxVersion` setting.
- the [MaterialX specification](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/documents/Specification)
  on MaterialX's `main` branch, which it downloads to find each node's section. Node hovers link there. If it can't be
  downloaded, the generator warns and the hovers have no links.

It also copies the skill for AI agents, `.claude/skills/shadinglanguagex`, into `skills/`. VS Code requires the
skill's folder to be named after the `name` in its `SKILL.md`, so the script stops with an error if they differ.

It also writes the colours SLX uses in VS Code's default themes into `package.json`, from `PLAYGROUND_COLORS`
and `SCOPE_COLORS` in the script. These match the MaterialX Playground's code editor, so update them if its colours
change, except for `argument`, `builtin` and `macro`, the colours of argument names, standard library calls and
macros, which the Playground doesn't have. If the grammar gains a scope that `SCOPE_COLORS` doesn't cover, the script stops with an error.

Run it after any of these change, or after changing the grammar template.

### Building and testing

Build, bundle the compiler and install, from the repository root:

```
python tools/vscode_extension/gen_extension.py --generate --converter --install
```

To work on the extension, first generate `data/` and copy the compiler into `media/lib`, from the repository root:

```
python tools/vscode_extension/gen_extension.py --generate --converter
```

Then open `slx-vscode/` in VS Code and press F5. This builds the extension and opens a second VS Code window with
it loaded.

From `slx-vscode/`:

| Command | What it does |
| --- | --- |
| `npm ci` | Install the build tools |
| `npm run build` | Build `dist/` |
| `npm run watch` | Rebuild `dist/` on every change |
| `npm test` | Type-check, then run the unit tests and the grammar tests |
| `npm run package` | Package `../slx-<version>.vsix` |

The unit tests in `test/*.test.ts` run with Node's built-in test runner. The tests that compile SLX need the
compiler in `media/lib`, and those that use MaterialX node data need `data/`. Both are skipped when the files are
missing. The grammar tests in `test/grammar/*.test.mxsl` use
[vscode-tmgrammar-test](https://github.com/PanAeon/vscode-tmgrammar-test): each line starting with `//` and a `^`
checks the scopes of the characters above it.

CI builds, tests and packages the extension in the *Build JavaScript (WASM)* workflow, and attaches the `.vsix` to
releases.
