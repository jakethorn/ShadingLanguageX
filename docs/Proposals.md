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

### Exact error ranges

Mark each error at its exact column and range, rather than the whole line. The compiler reports every error of a
compile with its file and line, but tokens don't record a column. The scanner could track columns (a token expanded
from a macro would take the position where the macro is used) and the compiler could pass them on with each error.

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

## Known Bugs

Bugs that have been found but not fixed yet.

### Node expressions are folded with their arguments in the written order

When every argument of a node expression is known at compile time and its category is one of the nodes the
compiler can evaluate, such as `add`, `divide` or `ifgreater`, the compiler evaluates it instead of writing the node.
This happens with the `comptime` modifier, and also without it, because graph reduction is on by default.
`serialize_constexpr` passes the argument values to the evaluating function in the order they're written, instead of
matching them to the node's inputs by name:

- `comptime node divide(in2=4.0, in1=2.0)` evaluates to `2.0`, but `comptime divide(in2=4.0, in1=2.0)` gives `0.5`.
- `node ifgreater(value2=1.0, value1=2.0, in1=10.0, in2=20.0)` evaluates to `20.0` instead of `10.0` with the
  default compile options.
- `comptime node add(a=1.0, b=2.0)` evaluates to `3.0`, although `a` and `b` aren't inputs of `add`.
- `comptime node add(in1=1.0)` reads past the end of the argument values, because `wrap` in
  `serialize_constexpr_wrapper.h` indexes them without checking how many there are. It currently fails with
  "Binary operator + not supported for values of type 'float' and 'void'".
- `float b = node add<float>(in1=1, in2=1);` fails with "Comptime expressions evaluated to a variable of type
  integer, not float", although it doesn't use `comptime`. Without graph reduction the node is written instead.

A fix would match the arguments to the inputs of the node's NodeDef by name and fill in missing inputs with their
default values, as function calls already do, and would skip evaluating the node if an argument doesn't name one of
its inputs. Outside of `comptime`, a result of the wrong type could also write the node instead of failing. Evaluating
node expressions can't be removed, because the standard library relies on it, e.g., `__not__` returns
`node not(in=in1)`. The code is in `serialize_constexpr.cpp`.

### Evaluating a method of a class instance crashes

[methods010.mxsl](../mxslc++/tests/data/groundtruth/classes/methods010.mxsl) crashes with a segmentation fault when
it's compiled with graph reduction, which is on by default, e.g., with the CLI. The ground-truth tests compile with
`reduce_graph = false`, so they don't catch it, and `--no-reduce-graph` compiles it correctly.

```
class A
{
    float x, y;

    float add()
    {
        return x + y;
    }
}

A a = {1.0, 2.0};
constant(a.add());
```

The crash is a null pointer read in `primitive_utils::add`, called by `serialize_constexpr` when
`Serializer::write_node` evaluates the method call at compile time. It looks like the instance's fields don't reach
the evaluation as compile-time values.

### User functions named after MaterialX nodes are evaluated as those nodes

[library_names001.mxsl](../mxslc++/tests/data/groundtruth/misc/library_names001.mxsl) defines its own `invert`
function. With graph reduction, which is on by default, it fails with "Cannot invert a value of type 'float'", and
"Cannot invert a value of type 'vector2'" for the `vec2` call. The ground-truth tests compile with
`reduce_graph = false`, so they don't catch it.

```
T invert<float, vec2>(T in)
{
    T result = in * -1.0;
    return result;
}

float y = invert(0.25);
vec2 z = invert(vec2{1.0, 2.0});
```

The error is thrown in `primitive_utils/invert.cpp`. It looks like `serialize_constexpr` picks the evaluating function
by the node's category, so it evaluates the user's function as MaterialX's `invert` node instead of through its own
body. User functions with the names of other MaterialX nodes, such as `add` or `multiply`, may be affected in the
same way.
