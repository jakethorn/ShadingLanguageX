# Proposals

Ideas that haven't been implemented yet.

## VSCode Extension

Improvements to the VS Code extension in [tools/vscode_extension](../tools/vscode_extension). Apart from
publishing, each one needs changes in the compiler or its WebAssembly bindings.

### Publish to the VS Code Marketplace

List the extension in the VS Code Marketplace, so it can be installed from the Extensions view instead of from a
`.vsix` file. This needs a `shadinglanguagex` publisher account and an access token, after which
`gen_extension.py --publish` uploads it. The extension also needs an icon, a 128×128 PNG. Publishing to Open VSX as
well would reach Cursor and VSCodium users.

### More compile settings

Let the extension set the entry function and its arguments, values for globals, and macros. The compiler already
supports these, but the WebAssembly bindings don't expose them. Today, globals compile with their initial value or
zero, and there's no way to choose an entry function.

### Highlight macros from the settings

Once macros can be set in the settings, highlight them where they're used, as macros defined with `#define` in the
file or its includes already are. The extension would take the names from the setting, so this needs nothing from
the compiler beyond [More compile settings](#more-compile-settings).

### Better errors

Report errors with their exact column and range, rather than marking the whole line, and report more than one error
per compile. The compiler stops at the first error, so a file with several mistakes shows them one at a time.

### Fix the build-up after failed compiles

After about 1500 failed compiles in one compiler instance, each successful compile leaks about 250 KB and gets
slower, until the compiler is unusable. Checking a file as it is typed fails often, so the extension works around
this by restarting the compiler after every 200 failed compiles. Fixing it in the compiler would remove the
workaround.

### Constructor parameters with the same name as a field

A class constructor whose parameter has the same name as a field fails with "Variable already defined". The `Ray`
class example in the language specification also fails, with "No function could be found with the name 'Ray'".

### Globals with initial values

With `error_on_missing_globals` on, a global with an initial value, such as `global float g = 0.5;`, still reports an
error. If that isn't intended, globals with initial values should compile with that value. The extension's
`slx.errorOnMissingGlobals` setting is off by default because of this.

### Reset the include folder after nested includes

When the source is compiled from a string, as the extension does, the folder that relative `#include` paths are
resolved against isn't reset after a nested include. A later relative include can then be looked for in the wrong
folder. This is rare.

### Symbols API and a language server

Have the compiler report the declarations it finds, with their types, through a symbols API, and build a language
server on it. Today the extension matches names without types, so completion, hovers and Go to Definition can't
choose between overloads, can't complete after a `.`, and list every method with a given name. A language server
would fix those, add rename and find references, and work in editors other than VS Code.

### Type hints

Show the inferred type of `auto` variables, and make parameter hints show the overload that applies to a call. Both
need type information from the compiler.

### Map source lines to nodes

Link each line of SLX to the MaterialX nodes it produces, so clicking a line highlights its nodes in the preview,
and clicking a node shows the line that made it. This needs the compiler to record where each node comes from.

### Step-through compile debugging

Step through a compile in VS Code's debugger, to see how each statement turns into nodes. It would build on the
compiler's existing `debug::Debugger`.

## Compiler

Improvements to the compiler in [mxslc++](../mxslc++). Both also affect the decompiler.

### Extreme int and float literals

The compiler can't read two kinds of valid literals. MaterialX documents can hold these values, so the decompiler
writes them, but the code it writes doesn't compile:

- `-2147483648`, the smallest `int`, fails with the bare message `stoi`. The `-` is parsed as a unary operator on
  `2147483648`, which is too large for an `int`. Folding a unary `-` in front of an int literal into the literal
  would fix it. Today the only way to write the value is `-2147483647 - 1`, which adds a `subtract` node.
- Denormal floats, such as `1.0e-40`, fail with the bare message `stof`. `std::stof` throws when its result is
  denormal, but `std::strtof` accepts it.

Literals that really are out of range, such as `2147483648`, should report a clear error instead of `stoi` or
`stof`. Both conversions are in `Token::literal`.

### Save files into new directories

`compile_to_file`, `decompile_to_file` and the CLI's `-o` fail with "Cannot save file: <path>" when the destination's
directory doesn't exist. The message is the same when the directory can't be written to, so it doesn't say what went
wrong. Saving could create the missing directories, and the error could give the reason, such as "No such
directory" or "Permission denied". All of them save through `io_utils::save_file`.
