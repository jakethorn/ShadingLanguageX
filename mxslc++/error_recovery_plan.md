# Compiler error recovery plan

Goal: a compile with several mistakes reports all of them, not just the first. The other half of the "Better errors"
proposal in `docs/Proposals.md` (exact columns and ranges) is a separate, optional stage at the end.

## How errors work today

- About 210 `throw CompileError{...}` sites. The first one thrown unwinds out of `compile()`.
- The location is added on the way out: the innermost `Statement::execute`, `Expression` (`TRY_END`) or
  `Parser::parse` calls `set_debug_info(token)`, which only sets it if it isn't set yet. Tokens have a line but no
  column. The location is stored as a preformatted `"file, line N: "` string.
- Phases: scan, preprocess, parse the whole file into statements, then execute them. Execution is an interpreter
  that builds the graph:
  - nodedef/nodegraph function bodies run once, at the definition (`Serializer::write_node_def_graph`)
  - inline function bodies run at every call
  - loops are unrolled, so loop bodies run once per iteration
  - templates are copied (monomorphized) for each type, so each copy runs separately
- Speculation: overload resolution calls `Expression::try_init` for each candidate and catches
  `AmbiguousFunctionError`. The message is kept as a string (`error_message_`) and thrown again later as a plain
  `CompileError`, so the error's type is lost. `is_function_resolution_error` sorts errors by checking whether the
  message starts with "Cannot assign".
- Paired state changes are not exception-safe:
  - `enter_scope`/`exit_scope` in `compile.cpp`, `IfStatement`, `ForEachLoop`, `FunctionCall`, `MethodCall` and
    `Serializer::write_node_def_graph`
  - `begin_comptime`/`end_comptime` in `Expression::evaluate`, `Function::invoke`, `FunctionCall` (twice),
    `IfStatement` and `IndexingOperator`
  - `ReturnStatement` exits scopes itself, then throws `Branch`

  This is fine today because any error ends the compile. Once the compile carries on after an error, an error in
  `x = inline_func(bad);` would leave the runtime inside `inline_func`'s scope.
- Consumers:
  - CLI: `main.cpp` catches nothing, so a compile error escapes `main` and calls `std::terminate`.
  - Python: pybind11's default translation turns errors into `RuntimeError`.
  - JS: `rethrow_as_js_error` creates an `Error` carrying `what()`.
  - VS Code extension: `compileErrors.ts` reads the location out of the text with a regex
    (`^(?:(.+?), )?line (\d+): ...`) and marks the whole line.

## Design

### 1. `Diagnostic`, and `CompileError` as a list of them

```cpp
enum class Severity { Error, Warning };

struct SourceLocation
{
    SharedPathPtr path;
    size_t line{0};
    size_t column{0};   // 0 = unknown until the column stage
};

struct Diagnostic
{
    Severity severity{Severity::Error};
    SourceLocation location;
    string message;

    string to_string() const;   // "file, line N: message", the same text as today's what()
};
```

- `CompileError` keeps all of its current constructors, so none of the 210 throw sites change.
- Inside, it holds `vector<Diagnostic>`: one entry when thrown at a throw site, many when thrown at the end of a
  compile.
- `what()` joins the entries with `"\n"`. A compile with one error produces exactly today's text, so the
  `error_message` ground truth and the extension's regex keep working.
- `set_debug_info(token)` keeps today's rule: it only sets a location that isn't set yet. It now stores a
  `SourceLocation` instead of a string, and the text is formatted only in `to_string`.
- New: `diagnostics()`.

New subclasses of `CompileError`:

- `SuppressedError`: "this statement can't run because of an error already reported". It is never reported (see 3.3).
- `FatalError`: stops the compile. Used for too many errors, unexpected end of tokens, standard library not found,
  and runtime not created.

`SuppressedError` must not derive from `AmbiguousFunctionError`, so that `try_init` doesn't catch it.

### 2. `ErrorReporter`

One class, used by each phase that recovers:

```cpp
class ErrorReporter
{
public:
    void report(const CompileError& e);   // appends e's diagnostics; skips exact duplicates (same location and message)
    bool has_errors() const;
    void throw_if_errors() const;         // throws CompileError{diagnostics_} if there are any
    const vector<Diagnostic>& diagnostics() const;
};
```

- `report` throws `FatalError` once `CompileOptions::max_errors` is reached. The default is 20, the same as clang,
  and 0 means no limit.
- Four users: the scanner (a local), `Preprocessor`, `Parser` and `Runtime` (members). `Runtime` is created after
  preprocessing, so no global reporter is needed.

### 3. Phases and gates

```
scan + preprocess the source      → throw_if_errors   (gate 1)
parse the source                  → throw_if_errors   (gate 2)
execute (recovering), entry func  → throw_if_errors   (gate 3)
finalise                          (only on success)
```

Each phase recovers inside itself and throws all of its errors at its end. So the gates come from the phase
functions throwing, and `compile.cpp` barely changes.

Why gate between parsing and execution: execution builds the graph. If it ran with statements missing, every name
those statements define would cause false "not defined" errors. Syntax errors are reported together, and the type
errors show once the file parses. A later relaxation is listed at the end.

### 4. Parser recovery

The parser loops over statements in three places: `Parser::parse` (top level), `block_statement` (bodies of
functions, if, else, for and constructors) and the body loop in `class_definition`. Merge them into one
statement-list helper and give it the recovery:

1. Catch `CompileError`, report it, then call `synchronize()`.
2. `synchronize()` skips tokens and counts `{}` depth. It stops at the first of:
   - a `;` at depth 0 (consume it)
   - a `}` that brings the depth back to 0 (consume it, but keep going if the next token is `else`)
   - a `}` at depth 0, which closes the enclosing block (leave it for the caller)
   - the end of the tokens
3. Always move forward: if the error happened on the statement's first token and nothing was skipped, consume one
   token.

Newlines are removed (`ignore(Newline)`), so the parser can't synchronize on line breaks.

### 5. Scanner and preprocessor recovery

Scanner:

- An unknown character is reported, skipped, and scanning continues.
- The message stays "Scanning error on line N, character: X" until the extension stops parsing error text.

Preprocessor: each directive is a recovery point. Report the error, skip to the end of the directive's line, and
carry on.

- An unknown directive, or a stray `#elif`, `#else` or `#endif`, is ignored.
- An unterminated `#if` is reported at the `#if` and closed at the end of the file.
- A missing `#include` is reported and skipped.
- Errors from an included file are added to the includer's reporter.

The standard library stays fatal, since an error there is a compiler bug.

### 6. Execution recovery (the core)

#### 6.1 Recovery point: `Statement::execute`

`Statement::execute` already wraps `init()` and `execute_impl()` and adds the location. It becomes the recovery
point for every statement at every depth: the top level, function bodies at the definition, inline bodies at the
call, loop and if bodies, and class members. An error in a function body then doesn't hide the rest of that body.

```cpp
const Checkpoint checkpoint = runtime().checkpoint();
try { /* as today */ }
catch (const FatalError&) { throw; }
catch (CompileError& e)
{
    e.set_debug_info(token_);
    runtime().restore(checkpoint);
    if (not is_suppressed(e))
    {
        errors().report(e);
        has_failed_ = true;
    }
    recover();
}
```

#### 6.2 Checkpoint and restore

Don't make each enter/exit pair RAII. Instead, the recovery point records what the pairs change and puts it back:

- `Runtime::checkpoint()` records the scope depth and the depth of the serializer's comptime stack.
- `Runtime::restore(cp)` exits scopes until it is back at the recorded depth, pops the comptime stack to its
  recorded size, and clears `comptime_violated_`.

`restore` only ever pops. `ReturnStatement` may already have exited past the checkpoint; then `restore` does
nothing and `ReturnStatement::recover()` takes over (see 6.4).

Why this rather than RAII: making every pair RAII means changing about 12 sites, and `ReturnStatement` would have to
rely on unwinding instead of exiting scopes itself. Checkpoint and restore keeps the guarantee in one place. A pair
added later only needs adding to `Checkpoint`.

#### 6.3 Stopping one error from causing more

When a statement fails, whatever it would have defined must still exist, or later statements report false errors.

- `Scope::poison(name)` marks a name as belonging to a failed definition.
- Lookups that would throw "not defined" call `throw_if_poisoned(name)` first. If the name is poisoned, it throws
  `SuppressedError`. The sites are:
  - `Scope::get_variable` (2 throws)
  - `Identifier` ("Variable or parameterless function not defined")
  - `Scope::resolve_type` ("Type ... not defined")
  - the end of `FunctionResolver::resolve`
- The check has to happen at the lookup. It can't be left to the recovery point, because function-resolution
  errors pass through `try_init` as strings and lose their type.
- The recovery point doesn't report a `SuppressedError`, but still calls `recover()`, so poisoning carries on down
  the chain: `auto y = x;` with a poisoned `x` poisons `y`.

#### 6.4 `Statement::recover()`

A new virtual function on `Statement`. By default it does nothing.

| Statement | `recover()` |
|---|---|
| `VariableDefinition` | If the type is resolved and isn't `auto`, define the variable with its declared type and a default value. This is the same path as `float x;` (`create_variable(type_)`), so later uses are still type-checked: `float x = bad; string s = x;` reports both errors. Otherwise (`auto`, or the type itself is undefined), poison the name. If the name is already defined, do nothing. |
| `MultiVariableDefinition` | Each name is a separate `VariableDefinition` statement, and each one recovers. If the right-hand side fails before those run, do the same as above for each name. |
| `FunctionDefinition` | Functions are added to the scope before their body is written, so after a body error the signature can still be called. Mark the function failed. A call to a failed function throws `SuppressedError`, because writing a call node against a half-written nodedef or nodegraph can crash. If the signature itself failed, poison the name. |
| `ClassDefinition` | If the type wasn't defined, poison its name. Members are statements, so they recover on their own. |
| `UsingDeclaration` | Poison the alias. |
| `ReturnStatement` | Throw a failed `Branch`. `Function::invoke` turns it into `SuppressedError` instead of "Non-void function did not return a value". The caller's statement is then skipped silently and runs its own `recover()`. In a nodedef or nodegraph body, this reaches the `FunctionDefinition` recovery point. |
| Others | Nothing. An assignment keeps the old value. An `if` whose condition fails is skipped without checking its bodies; this is fine for a first version. |

#### 6.5 Statements that run more than once

Inline bodies run at every call, loops once per iteration, and template copies once each.

- A statement that reported an error sets `has_failed_`. Each later run throws `SuppressedError` straight away, which
  then runs `recover()` again. This avoids duplicate errors. It also avoids running `init()` again on a statement
  that was only half initialised: for example, `FunctionCall::init_impl` moves `name_` into `method_call_` before
  `method_call_->init` can throw. The flag is only set for reported errors, not suppressed ones.
- `ErrorReporter` skips exact duplicates. This covers template copies, which are separate statement objects with the
  same token.
- Unchanged: an inline function that is never called is never checked.

#### 6.6 Speculation

No statement runs inside `try_init` today, so no recovery point can swallow an error from an overload being tried.
Keep it that way with an assertion: `try_init` increments a depth counter and `Statement::execute` asserts it is
zero. Don't build any buffering unless a case turns up.

#### 6.7 Other exceptions

Exceptions that aren't `CompileError` (`MaterialXValidateError`, `mx::Exception`, `std::out_of_range`, ...) go
straight through recovery points. At the top of `compile()`:

- If errors were already reported, throw those instead. The internal exception almost certainly comes from carrying
  on after them.
- If none were reported, rethrow the exception as today.

This way a user never sees an internal error in place of their real errors.

#### 6.8 After execution

1. Errors from invoking the entry function are reported the same way.
2. Reflection info is collected as today.
3. `throw_if_errors()` runs before `finalise()`. MaterialX validation of a document built after errors would only
   add noise.
4. `check_unused_globals` reports through `ErrorReporter`.

A useful side effect: with `error_on_missing_globals`, all missing globals are now reported in one compile, because
each `global` definition recovers with its default value.

### 7. Consumers

- **C++**: `compile()` still throws `CompileError`, which now also has `diagnostics()`.
- **CLI**: `main` catches `CompileError`, prints each diagnostic with `Logger::error`, and returns 1.
- **Python**: register `mxslc.CompileError`, a subclass of `RuntimeError` so that `except RuntimeError` and the
  existing `pytest.raises(RuntimeError, ...)` keep working. Give it `.diagnostics`, a list of
  `Diagnostic(severity, file, line, column, message)`, and regenerate `_mxslc.pyi`.
- **JS**: `rethrow_as_js_error` sets `err.diagnostics = [{severity, file, line, column, message}, ...]`.
- **VS Code extension**: pass `error.diagnostics` through the worker protocol and create one `vscode.Diagnostic` per
  entry. Keep the regex parser only for errors without diagnostics (decompile and internal errors).

### 8. Tests

- Existing `tests/data/error_message`: regenerate with `overwrite_data_files()` and review each diff. A new line is
  either a genuine second error (keep it) or a knock-on error (fix the suppression). Most of these files contain one
  error, so most won't change.
- New small files in `tests/data/error_message/recovery/` (each `.txt` lists every expected error). Each tests one
  behaviour:
  - two unrelated type errors: both reported
  - `float x = undefined_name; float y = x + 1;`: one error
  - `auto x = undefined_name; auto y = x;`: one error
  - `float x = bad; string s = x;`: two errors (declared-type fallback)
  - an error in an inline function called twice: one error
  - an error in a loop body: one error
  - a failed `return` in an inline function: no "did not return a value"
  - an error in a nodegraph body, then a call to it: one error
  - an error inside an inline call's argument, then a global definition that is used later: no false errors (the
    scope was restored)
  - `max_errors`
  - parser: two syntax errors in different statements: both reported
  - parser: a syntax error and a type error: only the syntax error (gate 2)
  - parser: an error inside an `if (...) {...} else {...}` header: no stray "Invalid statement" from `else`
  - preprocessor: two bad directives: both reported
- Python: one test for `CompileError.diagnostics`. JS: one test for `err.diagnostics`.
- `failure_tests` stay as they are (they still throw).

## Stages

Each stage leaves the tests passing and can be merged on its own.

1. **Foundations** (done): `Diagnostic` and `SourceLocation`, `CompileError` as a list, `ErrorReporter`,
   `FatalError`, and the CLI catch with an exit code. Behaviour doesn't change. Differences from the design above,
   to avoid unused code:
   - `Severity` and `SourceLocation::column` are added in stages 7 and 6.
   - `SuppressedError` is added in stage 2, its first user.
   - `ErrorReporter` takes its limit in its constructor. `CompileOptions::max_errors` is added in stage 2, when the
     runtime first owns a reporter.
   - `CompileError::message()` asserts there is exactly one diagnostic. Its only user is `try_init`, with
     `AmbiguousFunctionError`.
2. **Execution recovery** (done): checkpoint and restore, the recovery point, poisoning, `recover()` overrides,
   `has_failed_`, gate 3 and the top-level handling in `compile()`. Of the ground-truth files, only `switch112` gained
   an error, and it is a genuine one. The 14 files in `tests/data/error_message/recovery/` cover the behaviours listed
   in section 8. Differences from the design above:
   - Only statements with a source location are recovery points. Statements the compiler creates itself (the
     per-field definitions of a multi-variable definition, out-argument definitions) have no token. An error in one of
     them goes to the statement it belongs to, so it is reported once, with that statement's location.
   - `recover()` is public and const, so `MultiVariableDefinition::recover()` can run each field definition's
     `recover()`.
   - A failed function or constructor definition is removed from the scope and its name poisoned, instead of marking
     the function failed. A half-written nodegraph is then never called.
   - A failed `return` throws an empty `Branch`. `Function::invoke` then finds no return value, and "Non-void
     function did not return a value" becomes `SuppressedError` once any error has been reported. The same rule
     covers an `if` whose condition failed, which skips the function's only `return`.
   - A variable is given a default value only when its type is plain data, including in its fields
     (`type_utils::has_default_value`). A default `surfaceshader` has no value, and accessing its ports would cause a
     knock-on error. Other types are poisoned. If adding the default fails, for example because of an invalid
     modifier, the name is poisoned.
   - Also checks poisoning: `FieldAccessor`, which looks up parameterless methods by name.
   - No speculation assertion. Parameterless inline functions run their bodies in `Identifier::init_impl`, which
     runs inside `try_init`. Errors there are real errors in the body, so they are reported.
   - `check_unused_globals` stays in `finalise()`, after gate 3. A `global` whose definition failed was never added
     to reflection, so it would cause a false "unused global" error.
   - `CompileOptions::max_errors` (default 20) is C++ only so far. The CLI, Python and JS options move to stage 5.

   Known gaps:
   - An out-argument definition in a call that fails before its arguments are initialised (`typo(out float x);`)
     never defines `x`, so every later use of `x` is reported. Fixing this needs a failed statement to find the
     definitions nested in its expressions.
   - A template body is copied for each type. If its error message names the type, each copy reports its own error.
   - A `return` at the top level stops the compile: the `Branch` its `recover()` throws has no function to catch it.
3. **Parser recovery** (done): `parse_statement_or_skip()` is the recovery point. It is used by the top-level loop
   and by `statements_until_closing_brace()`, which is shared by blocks and class bodies. Gate 2 is
   `throw_if_errors()` between parsing and executing, in `parse_and_execute()`. No ground-truth file changed. The 14
   `parse_recovery` files in `tests/data/error_message/recovery/` cover it. Differences from the design above:
   - The parser reports to the runtime's `ErrorReporter`, which `parse()` and the `Parser` constructor now take. So
     a compile has one reporter and one error limit, and `ConstructorDefinition` passes the same one.
   - `skip_statement()` replaces `synchronize()`. It goes back to where the statement started and counts braces up
     to the point where parsing failed. Braces still open there are taken to be broken, for example
     `return vec3{1.0, 2.0;`. A `;` at their level ends the statement, and closing them doesn't. After that, the
     statement ends at:
     - a `;` outside braces;
     - a `}` that closes back to depth 0, unless the next token is `else` or can't start a statement (for example
       `)`, `,` or an operator, as in `dot(v, {0, 1, 0})`).

     Inside a block, it stops before a `}` at depth 0, which is taken to close the block. At the top level such a `}`
     is skipped as part of the statement, since there is no block for it to close.
   - Brackets aren't counted, so an unclosed `(` can't swallow the rest of the file.
   - `TokenReader` copies tokens when it consumes them instead of moving them. Tokens are then still intact when
     `skip_statement()` goes back over them, and "Unexpected end of tokens" now has its file name.
   - `function_body()` was the same as `block_statement()`, so it was removed. `parse_utils::parse_function()` had no
     callers, so it was removed rather than changed for the new `parse()`.

   A mutation test over the ground-truth files deleted one `;`, `(`, `)`, `{`, `}`, `,` or `=`, or inserted a `*`.
   Of the 1,133 mutants that failed to compile, 1,041 report one error. Most of the rest come from typos that are
   ambiguous to a brace-counting parser:
   - A missing `{` makes its `}` close a block early.
   - A `}` inside a block is taken to close the block.
   - A typo that stops a definition parsing as one (such as `Point p {1.0, 2.0};`) leaves the name undefined.

   A missing `;` swallows the next statement. A heuristic could use the line break, but none is used.
4. **Scanner and preprocessor recovery** (done):
   - **Scanner:** `scan_string()` still throws, so its other callers (`parse_cli_args`, `Token`, macros) keep
     working. The error it throws lists every run of unexpected characters as "Unexpected character: X", located at
     its line. This replaces "Scanning error on line N, character: X".
   - **Preprocessor:** each directive is a recovery point (`process_directive_or_skip_line()`). The error is reported
     at the directive's `#`, and the rest of the line is skipped.
     - A failing `#if`, `#elif`, `#ifdef` or `#ifndef` condition is reported and treated as false, so the
       `#else`/`#endif` structure stays intact.
     - Errors in an included file, including its scan errors, are reported and the includer carries on.
     - After an unknown directive (perhaps a misspelled `#ifdef` or `#endif`), errors about unmatched conditional
       directives in that file are suppressed.
     - "File ... could not be found", circular includes and "Unterminated #if directive" now have locations.
   - **Gate 1:** `compile()` gives the preprocessor its own `ErrorReporter`, because the runtime is only created after
     preprocessing (`#version` and `#library` change the options it is created with). `throw_if_errors()` comes
     before `Runtime::create()`.
   - **Removed:** the four unused `preprocess()` overloads.
   - **Tests:** three ground-truth files changed. Two now have locations, and `bad_directive003`'s `#endif` is also
     reported, since neither directive has an `#if`. The 8 `scan_recovery`/`preprocess_recovery` files cover the rest.
   - **Mutation test:** a third set inserts a `$` at a random position. 751 of 772 mutants report one error.

   Still without a location: an invalid `#version` and a missing `#library` file. Both are only checked when the
   runtime is created.
5. **Consumers** (done):
   - **CLI:** `--max-errors N`, with help text and `parse_args_tests`.
   - **Python:** `mxslc.CompileError` (a `RuntimeError`) with `.diagnostics`, a list of `mxslc.Diagnostic` (`file`,
     `line`, `message`, `str()`). It is registered with `py::gil_safe_call_once_and_store` and an exception
     translator. `CompileOptions.max_errors` is a field and a constructor keyword, and the stubs were regenerated.
     The stubs can't show `CompileError.diagnostics`, because it is set when the error is raised.
   - **JS:** a compile or reflect error has `.diagnostics`, an array of `{file, line, message}`, and
     `CompileOptions.maxErrors` is new.
   - **Extension:** `EngineFailure` and `EngineError` carry `diagnostics`, and `locateCompileErrors()` places each one
     with the existing logic. The message is only parsed when there are no diagnostics. The "Scanning error on line"
     case was removed.
   - **Docs:** the User Guide (Max Errors), the API Documentation (Compile Errors), the JS README, the skill's
     compiler reference, and `Proposals.md` ("Better errors" became "Exact error ranges").
6. **Optional, columns**: the scanner tracks columns (tokens expanded from a macro take the position where the
   macro is used), and `SourceLocation::column` is filled in. The extension then marks the exact token instead of
   the whole line. This completes "Better errors" in `Proposals.md`.
7. **Optional, warnings**: the three `Logger::warning` calls in the compiler (`Function.cpp`, `CompileOptions.cpp`,
   `mtlx_utils.cpp`) go through `ErrorReporter` with `Severity::Warning`, and the extension shows them as warnings.

## Open decisions

1. **Stop after syntax errors?** Recommended: yes (gate 2). The alternative is to execute anyway, replacing each
   failed statement with an `ErrorStatement` that poisons the names it would have defined. It reports more per
   compile, but there are more ways for one error to cause false ones.
2. **API shape?** Recommended: keep throwing one `CompileError` that carries every diagnostic. There's no API break,
   and `what()` is unchanged when there is one error. The alternative is a `CompileResult` return value, which means
   new APIs in C++, Python and JS.
3. **Default `max_errors`?** Recommended: 20, the same as clang. 0 means no limit.
4. **Columns and warnings now or later?** Recommended: later (stages 6 and 7). Neither is needed for multiple errors.

## Later

- An `if` whose condition fails could still check its bodies.
- Error kinds or codes instead of matching message prefixes (`is_function_resolution_error`).
- Measure whether compiles that recover reduce the "build-up after failed compiles" leak in `Proposals.md`. Scopes
  are now exited normally after most errors.
