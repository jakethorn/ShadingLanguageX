# ShadingLanguageX (SLX) for VS Code

Language support for [ShadingLanguageX](https://github.com/jakethorn/ShadingLanguageX), the shading language that
compiles to MaterialX node graphs.

## Features

- **Highlighting** for `.mxsl` files: keywords, types, MaterialX nodes, your own functions and classes, attributes,
  modifiers and preprocessor directives.
- **Errors as you type.** Open SLX files are compiled after each pause in typing, and errors appear in the Problems
  panel. An error in an included file is shown in that file and on the `#include` line that pulls it in.
- **Compile and decompile.** *SLX: Compile to MaterialX* saves the compiled MaterialX to a file you choose,
  starting with a `.mtlx` file of the same name next to the source. After that it updates the same file without
  asking, including in any editor it's open in. *SLX: Compile to MaterialX As...* saves to a different file.
  *SLX: Decompile to SLX* opens the SLX of a `.mtlx` file beside it.
- **Completion** of variables, your own functions and classes, MaterialX nodes, standard library functions, types
  and keywords. Inside a call, the names of the function's parameters or the node's inputs are offered as
  `name = `, leaving out those already passed.
- **Hovers** show the declaration and docs of variables and functions, including those in included files. For
  MaterialX nodes they show the signatures, inputs, defaults and allowed values, and link to the node's section of
  the [MaterialX specification](https://github.com/AcademySoftwareFoundation/MaterialX/tree/main/documents/Specification).
  Hover over an argument name, like `base_color` in `standard_surface(base_color = c)`, to see that input.
- **Parameter hints** show the signatures of the function or node being called, with the current argument
  highlighted.
- **Go to Definition** (Ctrl+Click, or F12) on a variable, function, class or type alias jumps to where it is
  declared, in the same file or an included one. On a standard library function such as `combine`, it opens the
  function in the extension's copy of `stdlib.mxsl`.
- **Links.** Ctrl+Click on an `#include` or `#library` path opens the file. With `slx.links.nodes` on, Ctrl+Click
  on a MaterialX node in a call, such as `mix` in `mix(a, b, t)`, opens the node's section of the MaterialX
  specification in your browser. It's off by default because VS Code underlines every link.
- **Outline and breadcrumbs** list functions, classes and their members, type aliases, globals, constants and
  material and shader variables.
- **Live MaterialX preview.** *SLX: Open MaterialX Preview to the Side* (Ctrl+K V) shows the compiled MaterialX
  beside the source, and updates it as you type.
- **Status bar.** The MaterialX version the file compiles against, and whether it compiles. Click it to choose the
  version.
- **Snippets** for materials, entry functions, functions, classes, loops, `if` and `switch` expressions, globals,
  `#include` and `#library`.
- **Includes and libraries.** `#include` and `#library` find files next to the file containing the directive, then
  in the folders in `slx.includePaths`. Unsaved changes in open editors are used.

Completion, hovers, parameter hints and the MaterialX version follow the file's `#version` directive, or the
`slx.materialxVersion` setting.

## AI agents

The extension includes a skill that teaches AI agents SLX: the language, the standard library, the compiler and
how SLX becomes MaterialX nodes.

- **GitHub Copilot in VS Code** uses it automatically, in recent versions of VS Code.
- **Claude Code and other agents** read skills from folders. Run *SLX: Install AI Skill* to copy it into your
  personal skills folder (`~/.claude/skills`), for all your projects, or into a workspace's `.claude/skills`, to
  share it through the repository.

A copied skill doesn't update with the extension, and VS Code's chat uses it in place of the one in the extension.
After updating the extension, run *SLX: Install AI Skill* again: it shows which copies are out of date and
replaces them.

## Colours

In VS Code's default themes (Dark 2026, Light 2026, Dark Modern, Light Modern, Dark+ and Light+), SLX uses the colours
of the MaterialX Playground's code editor: keywords, types, directives, numbers, strings, comments and attributes are
coloured, and names, your own function calls and operators are plain text. Three things are coloured that the
Playground leaves plain: argument names, like `base_color` in `standard_surface(base_color = c)`, in a soft red;
calls of standard library functions, like `mix` and `standard_surface`, in a muted yellow; and macros in lavender.
Other themes use their own colours.

Macros are highlighted wherever they're used: in their `#define`, in `#ifdef`-style directives, and in code. That
includes macros defined in included files, and the compiler's own `__MAIN__`, `__INCLUDE__`, `__FILE__` and
`__LINE__`. This uses VS Code's semantic highlighting, which the default themes turn on; in other themes, turn on
`editor.semanticHighlighting.enabled` to see macros used in code.

Brackets aren't colour-coded by nesting level, again as in the Playground, in any theme. VS Code still highlights
the bracket that matches the one next to the cursor. Turn `slx.bracketPairColorization` on to colour-code them.

To change a colour, add a rule for its scope to `editor.tokenColorCustomizations` in your settings, under the
theme's name. *Developer: Inspect Editor Tokens and Scopes* shows the scope of the text under the cursor. A
`textMateRules` list of your own for one of the default themes replaces the SLX rules for that theme.

The compiler is bundled with the extension as WebAssembly. Nothing else needs installing, and no network access is
needed.

## Commands

The buttons in the editor title bar and the commands in the editor's right-click menu change with the open file.

| Command | Files | What it does |
| --- | --- | --- |
| SLX: Open MaterialX Preview to the Side (Ctrl+K V) | `.mxsl` | Opens the compiled MaterialX beside the file, and keeps it up to date |
| SLX: Compile to MaterialX | `.mxsl` | Compiles the file and saves the MaterialX. The first time, it asks where, starting with a `.mtlx` file of the same name next to the source. After that it updates the same file. |
| SLX: Compile to MaterialX As... | `.mxsl` | Compiles the file and asks where to save the MaterialX. Later compiles update the file you chose. |
| SLX: Decompile to SLX | `.mtlx` | Decompiles the file and opens the SLX beside it |
| SLX: Select MaterialX Version | `.mxsl` | Sets `slx.materialxVersion`. Also runs when you click the status bar item. |
| SLX: Install AI Skill | any | Copies the SLX skill for AI agents into your personal or a workspace's skills folder |

Text printed by `print` statements and compiler warnings appear in the **SLX** output panel when you run a
command.

## Settings

| Setting | Default | What it does |
| --- | --- | --- |
| `slx.materialxVersion` | `1.39.5` | MaterialX version to compile against. A `#version` directive in the file takes precedence. |
| `slx.reduceGraph` | `true` | Simplify the compiled node graph. |
| `slx.defaultFunctionBehaviour` | `nodedef` | How functions without a `nodedef`, `nodegraph`, `inline` or `comptime` modifier compile. |
| `slx.errorOnMissingGlobals` | `false` | Report an error for every `global` variable. The extension can't pass values for globals, so when this is off they compile with their initial value, or zero. |
| `slx.includePaths` | `[]` | Extra folders to search for `#include` and `#library` files. Relative paths are resolved against the workspace folder. |
| `slx.links.nodes` | `false` | Make MaterialX node names in calls links to the node's section of the MaterialX specification, opened with Ctrl+Click. |
| `slx.bracketPairColorization` | `false` | Colour-code matching brackets by their nesting level. Turning it on or off sets `editor.language.colorizedBracketPairs` for SLX in the same settings file. |
| `slx.diagnostics.trigger` | `onType` | When to check open files: `onType`, `onSave`, or `off` (only when the file is compiled or decompiled with a command). |

## Limitations

- Only the first error in a file is shown, and it covers the whole line: the compiler stops at the first error and
  reports a line but not a column.
- Values for `global` variables, entry functions and their arguments, and macros can't be set yet.
- Completion, hovers and Go to Definition match names, not types. Overloaded functions show every overload, a
  method or field after a `.` matches every class member with that name, and nothing is completed after a `.`.
- The MaterialX preview keeps checking its source as you type, even when `slx.diagnostics.trigger` is `onSave` or
  `off`.

## Upgrading from MXSL Language Support

This extension replaces *MXSL Language Support* (`shadinglanguagex.mxsl-language`). Uninstall that extension, as
both claim `.mxsl` files. Settings and commands now start with `slx.` instead of `mxsl.`.

## Acknowledgments

This extension bundles the MaterialX standard libraries (MaterialX, Copyright Contributors to the MaterialX
Project), which are licensed under the Apache License, Version 2.0. See <https://www.materialx.org>.

## License

Apache-2.0
