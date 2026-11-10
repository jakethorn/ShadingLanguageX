# Changelog

## 0.4.0

- Completion of variables, functions, classes, MaterialX nodes, standard library functions, types and keywords,
  and of argument names inside calls.
- Hovers for variables, functions and MaterialX nodes, including node inputs passed by name. Node hovers link to
  the node's section of the MaterialX specification.
- Parameter hints for functions and MaterialX nodes.
- Go to Definition (Ctrl+Click, F12) for variables, functions, classes and type aliases, in the file and its
  includes, and for standard library functions. `#include` and `#library` paths are links to their files.
- With the new `slx.links.nodes` setting on, MaterialX node names in calls are links to the node's section of the
  MaterialX specification, opened with Ctrl+Click. It's off by default.
- Outline and breadcrumbs.
- *SLX: Open MaterialX Preview to the Side* (Ctrl+K V): a live view of the compiled MaterialX.
- The `slx.compileOnSave` setting writes the compiled MaterialX next to the source when it is saved.
- In VS Code's default themes, SLX uses the colours of the MaterialX Playground's code editor, plus a soft red for
  argument names, like `base_color` in `standard_surface(base_color = c)`, a muted yellow for calls of standard
  library functions, and lavender for macros.
- Macros are highlighted wherever they're used, including those defined in included files.
- Brackets are no longer colour-coded by nesting level, as in the Playground. The new `slx.bracketPairColorization`
  setting turns it back on.
- A status bar item shows the MaterialX version and whether the file compiles. Clicking it runs the new
  *SLX: Select MaterialX Version* command.

## 0.3.0

- Renamed from *MXSL Language Support* to *ShadingLanguageX (SLX)*. The language id is now `slx`, and commands and
  settings start with `slx.`.
- Errors are shown in the Problems panel as you type. Errors in included files are shown in those files and on the
  `#include` line.
- `#include` and `#library` now work. Before, the bundled compiler couldn't read files, so any file that included
  another failed to compile.
- Added settings for the MaterialX version, graph reduction, default function behaviour, missing globals, include
  folders and when files are checked.
- Split *Convert between MXSL and MTLX* into *SLX: Compile to MaterialX* and *SLX: Decompile to SLX*.
- `.mtlx` files open as XML, and the compile and decompile buttons appear only on the files they apply to.
- Rewrote highlighting: your own functions, classes and type aliases, attributes, modifiers, operators, all number
  formats and preprocessor directives are now highlighted, and themes choose the colours.
- Added snippets, code folding for `#ifdef` blocks and `// region` comments, and comment continuation.
- The compiler runs on a worker thread, so compiling large files doesn't slow down VS Code.
- `print` output and compiler warnings appear in the **SLX** output panel.

## 0.2.1

- Highlighting for `.mxsl` files.
- Commands to convert and validate between MXSL and MTLX.
