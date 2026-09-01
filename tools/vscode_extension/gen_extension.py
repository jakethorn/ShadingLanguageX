#!/usr/bin/env python3
"""Generate and/or install a VS Code extension for MXSL syntax highlighting.

Usage:
    python gen_extension.py --generate     # generate extension files
    python gen_extension.py --install      # install to ~/.vscode/extensions/
    python gen_extension.py --generate --install   # both
    python gen_extension.py --generate --converter   # also bundle the MXSL<->MTLX
                                                     # WebAssembly converter
    python gen_extension.py --generate --converter --build-js  # ... and build WASM
                                                     # if artifacts are missing
    python gen_extension.py --publish                # package + publish via vsce
                                                     # (needs vsce + marketplace PAT)
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import zipfile
from pathlib import Path

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
REPO_ROOT = Path(__file__).resolve().parent.parent.parent
KW_PATH = REPO_ROOT / "mxslc" / "mxslc" / "Keyword.py"
STDLIB_PATH = REPO_ROOT / "mxslc++" / "libraries" / "stdlib.mxsl"
OUT_DIR = REPO_ROOT / "tools" / "vscode_extension" / "mxsl-vscode"

# JavaScript (WebAssembly) build artifacts for the MXSL <-> MTLX converter.
JS_BUILD_BIN = REPO_ROOT / "mxslc++" / "javascript" / "build" / "mxslc" / "javascript" / "bin"
JS_BUILD_SCRIPT = REPO_ROOT / "mxslc++" / "javascript" / "build_javascript.sh"


# ---------------------------------------------------------------------------
# Keyword extraction
# ---------------------------------------------------------------------------

def load_keywords() -> dict:
    """Load MXSL keywords from Keyword.py (data types vs control)."""
    import importlib.util

    print(f"......Loading keywords from {KW_PATH}")

    spec = importlib.util.spec_from_file_location("Keyword", str(KW_PATH))
    mod = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(mod)

    data_types = sorted({str(k) for k in mod.Keyword.DATA_TYPES()})
    all_kw = sorted({str(k) for k in mod.Keyword})
    control = sorted(set(all_kw) - set(data_types))

    return {"data_types": data_types, "control": control}


def load_builtins() -> list[str]:
    """Load built-in function names from stdlib.mxsl."""
    if not STDLIB_PATH.exists():
        return []
    with open(STDLIB_PATH) as f:
        content = f.read()
    funcs = set(re.findall(r'\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(', content))
    funcs = {f for f in funcs if not f.startswith("__")}
    return sorted(funcs)


def package_vsix(src: Path) -> Path:
    """Create a .vsix package from the generated extension files."""
    pkg = json.loads((src / "package.json").read_text())
    version = pkg.get("version", "0.0.0")
    vsix_path = src.parent / f"mxsl-language-{version}.vsix"
    with zipfile.ZipFile(vsix_path, "w", zipfile.ZIP_DEFLATED) as zf:
        for file in src.rglob("*"):
            if file.is_file():
                arcname = "extension/" + str(file.relative_to(src))
                zf.write(file, arcname)
    return vsix_path


def publish_extension() -> None:
    """Package with vsce and publish to the VS Code Marketplace."""
    if shutil.which("vsce") is None:
        print("error: 'vsce' not found. Install it with: npm install -g @vscode/vsce")
        return
    print("\nRunning 'vsce package' ...")
    r = subprocess.run(["vsce", "package"], cwd=OUT_DIR, check=False)
    if r.returncode != 0:
        print("error: 'vsce package' failed; fix the reported issues and retry.")
        return
    print("Running 'vsce publish' ...")
    r = subprocess.run(["vsce", "publish"], cwd=OUT_DIR, check=False)
    if r.returncode != 0:
        print("error: 'vsce publish' failed. Ensure VSCE_PAT is set and the")
        print("       publisher is registered, then retry.")
        return
    print("\nPublished successfully.")


def load_mtlx_nodedefs(mtlx_stdlib_path: Path | None = None) -> tuple[list[str], str]:
    """Load MTLX nodedef names from MaterialX stdlib.
    
    Args:
        mtlx_stdlib_path: Optional directory containing MTLX stdlib libraries.
                          If None, uses the installed MaterialX package's built-in search paths.
    """
    import MaterialX as mx
    stdlib = mx.createDocument()

    if mtlx_stdlib_path:
        search_path = mx.FileSearchPath(str(mtlx_stdlib_path))
        lib_folders = list(mx.getDefaultDataLibraryFolders())
        mx.loadLibraries(lib_folders, search_path, stdlib)
    else:
        search_path = mx.getDefaultDataSearchPath()
        lib_folders = list(mx.getDefaultDataLibraryFolders())
        mx.loadLibraries(lib_folders, search_path, stdlib)

    unique_names = set(nd.getNodeString() for nd in stdlib.getNodeDefs())
    return sorted(unique_names), mx.getVersionString()


# ---------------------------------------------------------------------------
# JavaScript (WebAssembly) converter bundling
# ---------------------------------------------------------------------------

def run_js_build(args) -> None:
    """Invoke javascript/build_javascript.sh to produce the JsMxslc WASM artifacts."""
    if not JS_BUILD_SCRIPT.exists():
        print(f"  Warning: build script not found at {JS_BUILD_SCRIPT}")
        return

    emsdk = args.emsdk_location or str(REPO_ROOT.parent / "emsdk")
    mtlx_root = args.materialx_root or os.environ.get("MATERIALX_ROOT", "")

    print("  Running javascript/build_javascript.sh ...")
    cmd = ["bash", str(JS_BUILD_SCRIPT)]
    if emsdk:
        cmd.append(emsdk)
    if mtlx_root:
        cmd.append(mtlx_root)

    try:
        result = subprocess.run(cmd, cwd=REPO_ROOT, check=False)
        if result.returncode != 0:
            print(f"  Warning: build_javascript.sh exited with code {result.returncode}")
    except Exception as e:
        print(f"  Warning: could not run build_javascript.sh: {e}")


def resolve_js_build(args) -> Path | None:
    """Resolve the directory holding the JsMxslc WASM artifacts.

    Returns the ``bin`` directory when all three artifacts are present, otherwise
    attempts to build them (auto-build when missing, or a forced rebuild when
    ``--build-js`` is passed) and returns ``None`` if they still cannot be found.
    """
    bin_dir = Path(args.js_build_dir) if args.js_build_dir else JS_BUILD_BIN
    artifacts = ["JsMxslc.js", "JsMxslc.wasm", "JsMxslc.data"]

    present = all((bin_dir / a).exists() for a in artifacts)
    if not present or args.build_js:
        run_js_build(args)
        present = all((bin_dir / a).exists() for a in artifacts)

    return bin_dir if present else None


def copy_js_artifacts(bin_dir: Path) -> None:
    """Copy the JsMxslc JS/WASM/data artifacts into the extension's media/lib folder.

    The Emscripten module is an ES module; Node only treats it as ESM when the
    file carries a ``.mjs`` extension (the extension package is CommonJS), so the
    JS module is shipped under both names.
    """
    lib_dir = OUT_DIR / "media" / "lib"
    lib_dir.mkdir(parents=True, exist_ok=True)
    copies = [
        ("JsMxslc.js", "JsMxslc.js"),
        ("JsMxslc.js", "JsMxslc.mjs"),
        ("JsMxslc.wasm", "JsMxslc.wasm"),
        ("JsMxslc.data", "JsMxslc.data"),
    ]
    for src_name, dst_name in copies:
        src = bin_dir / src_name
        if not src.exists():
            print(f"  Warning: missing artifact {src.name}")
            continue
        shutil.copy2(src, lib_dir / dst_name)
        print(f"  -> {lib_dir / dst_name}")


# ---------------------------------------------------------------------------
# Generator
# ---------------------------------------------------------------------------

def escape_re(s: str) -> str:
    """Escape a string for use in a TextMate regex character class or alternation."""
    return re.escape(s)


def build_keyword_pattern(words: list[str]) -> str:
    """Build a case-sensitive word-boundary alternation pattern."""
    if not words:
        return r"\b(nonexistent)\b"
    escaped = [escape_re(w) for w in sorted(words, key=len, reverse=True)]
    return r"\b(?:" + "|".join(escaped) + r")\b"


def generate_package_json(with_converter: bool = False) -> dict:
    pkg = {
        "name": "mxsl-language",
        "displayName": "MXSL Language Support",
        "description": "Syntax highlighting and MXSL <-> MTLX (MaterialX) conversion for the MXSL Shading Language",
        "version": "0.2.1",
        "publisher": "shadinglanguagex",
        "license": "Apache-2.0",
        "repository": {
            "type": "git",
            "url": "https://github.com/kwokcb/ShadingLanguageX/"
        },
        "keywords": [
            "mxsl",
            "materialx",
            "shading language",
            "slx",
            "converter",
            "syntax highlighting"
        ],
        "engines": {"vscode": "^1.85.0"},
        "categories": ["Programming Languages"],
        "contributes": {
            "languages": [{
                "id": "mxsl",
                "aliases": ["MXSL", "mxsl"],
                "extensions": [".mxsl"],
                "configuration": "./language-configuration.json"
            }],
            "grammars": [{
                "language": "mxsl",
                "scopeName": "source.mxsl",
                "path": "./syntaxes/mxsl.tmLanguage.json"
            }],
            "configurationDefaults": {
                "editor.tokenColorCustomizations": {
                    "textMateRules": [
                        {
                            "scope": "entity.name.function.mxsl",
                            "settings": {"foreground": "#78C48B"}
                        },
                        {
                            "scope": "support.function.mxsl",
                            "settings": {"foreground": "#78C48B"}
                        }
                    ]
                }
            }
        }
    }

    if with_converter:
        pkg["main"] = "./src/extension.js"
        pkg["activationEvents"] = ["onCommand:mxsl.convert", "onCommand:mxsl.validate"]
        pkg["contributes"]["commands"] = [
            {
                "command": "mxsl.convert",
                "title": "MXSL: Convert between MXSL and MTLX"
            },
            {
                "command": "mxsl.validate",
                "title": "MXSL: Validate current file (MXSL <-> MTLX)"
            }
        ]
        pkg["contributes"]["menus"] = {
            "editor/context": [
                {
                    "command": "mxsl.convert",
                    "when": "editorLangId == mxsl || editorLangId == xml",
                    "group": "navigation"
                },
                {
                    "command": "mxsl.validate",
                    "when": "editorLangId == mxsl || editorLangId == xml",
                    "group": "navigation"
                }
            ]
        }

    return pkg


def generate_readme(with_converter: bool) -> str:
    """Return the extension README shown on the VS Code Marketplace."""
    lines = [
        "# MXSL Language Support",
        "",
        "Syntax highlighting for the MXSL (MaterialX Shading Language) files in",
        "Visual Studio Code.",
        "",
    ]
    if with_converter:
        lines += [
            "## Features",
            "- Syntax highlighting for `.mxsl` files",
            "- Convert the active `.mxsl` file to MaterialX XML (MTLX)",
            "- Convert the active `.mtlx` / `.xml` file to MXSL",
            "- Validate the active file without creating a new one",
            "",
            "## Commands",
            "",
            "| Command | Description |",
            "| --- | --- |",
            "| `MXSL: Convert between MXSL and MTLX` | Convert the open file and show the result in a new editor |",
            "| `MXSL: Validate current file (MXSL <-> MTLX)` | Run the conversion without creating a new file |",
            "",
            "Run these from the Command Palette (`Cmd/Ctrl+Shift+P`) or the editor",
            "context menu on a `.mxsl` / `.mtlx` file.",
            "",
            "## Conversion engine",
            "The converter uses the `mxslc` compiler/decompiler compiled to",
            "WebAssembly and bundled with the extension. No network access is required.",
            "",
            "## Acknowledgments",
            "This extension bundles the MaterialX standard library (MaterialX,",
            "Copyright (c) Contributors to the MaterialX Project), which is",
            "licensed under the Apache License, Version 2.0. See",
            "<https://www.materialx.org> for details.",
            "",
        ]
    lines += [
        "## Requirements",
        "- Visual Studio Code 1.85 or newer",
        "",
        "## License",
        "Apache-2.0",
        "",
    ]
    return "\n".join(lines)


def write_license() -> None:
    """Copy the repository Apache-2.0 LICENSE into the extension folder."""
    src = REPO_ROOT / "LICENSE"
    if not src.exists():
        print("  Warning: no LICENSE file found at repository root")
        return
    shutil.copy2(src, OUT_DIR / "LICENSE")
    print(f"  -> {OUT_DIR / 'LICENSE'}")


def generate_language_configuration() -> dict:
    return {
        "comments": {
            "lineComment": "//",
            "blockComment": ["/*", "*/"]
        },
        "brackets": [
            ["{", "}"],
            ["[", "]"],
            ["(", ")"]
        ],
        "autoClosingPairs": [
            {"open": "{", "close": "}"},
            {"open": "[", "close": "]"},
            {"open": "(", "close": ")"},
            {"open": "\"", "close": "\"", "notIn": ["string"]},
            {"open": "'", "close": "'", "notIn": ["string"]}
        ],
        "surroundingPairs": [
            {"open": "{", "close": "}"},
            {"open": "[", "close": "]"},
            {"open": "(", "close": ")"},
            {"open": "\"", "close": "\""},
            {"open": "'", "close": "'"}
        ]
    }


def generate_tm_grammar(kw: dict, builtins: list[str], functions: list[str]) -> dict:
    data_type_pat = build_keyword_pattern(kw["data_types"])
    control_pat = build_keyword_pattern(kw["control"])
    builtin_pat = build_keyword_pattern(builtins)
    func_pat = build_keyword_pattern(functions)

    return {
        "scopeName": "source.mxsl",
        "name": "MXSL",
        "fileTypes": ["mxsl"],
        "patterns": [
            # Preprocessor directives
            {"match": r"#\w+", "name": "keyword.control.directive.mxsl"},

            # Multi-line comment
            {"begin": r"/\*", "end": r"\*/", "name": "comment.block.mxsl"},

            # Single-line comment
            {"match": r"//.*$", "name": "comment.line.double-slash.mxsl"},

            # String literals
            {"match": r'"(?:[^"\\]|\\.)*"', "name": "string.quoted.double.mxsl"},
            {"match": r"'(?:[^'\\]|\\.)*'", "name": "string.quoted.single.mxsl"},

            # Numbers (float with scientific notation)
            {"match": r"\b\d+\.?\d*(?:[eE][+-]?\d+)?[fF]?\b", "name": "constant.numeric.mxsl"},

            # MTLX nodedef function names (only when followed by `(`)
            {"match": func_pat + r"(?=\s*\()", "name": "entity.name.function.mxsl"},

            # Built-in function names (only when followed by `(`)
            {"match": builtin_pat + r"(?=\s*\()", "name": "support.function.mxsl"},

            # Data types
            {"match": data_type_pat, "name": "storage.type.mxsl"},

            # Control keywords
            {"match": control_pat, "name": "keyword.control.mxsl"},
        ],
        "repository": {
            "comment": {
                "patterns": [
                    {"begin": r"/\*", "end": r"\*/", "name": "comment.block.mxsl"},
                    {"match": r"//.*$", "name": "comment.line.double-slash.mxsl"}
                ]
            },
            "string": {
                "patterns": [
                    {"match": r'"(?:[^"\\]|\\.)*"', "name": "string.quoted.double.mxsl"},
                    {"match": r"'(?:[^'\\]|\\.)*'", "name": "string.quoted.single.mxsl"}
                ]
            },
            "number": {
                "match": r"\b\d+\.?\d*(?:[eE][+-]?\d+)?[fF]?\b",
                "name": "constant.numeric.mxsl"
            },
            "function": {
                "match": func_pat + r"(?=\s*\()",
                "name": "entity.name.function.mxsl"
            },
            "builtin": {
                "match": builtin_pat + r"(?=\s*\()",
                "name": "support.function.mxsl"
            },
            "type": {
                "match": data_type_pat,
                "name": "storage.type.mxsl"
            },
            "control": {
                "match": control_pat,
                "name": "keyword.control.mxsl"
            },
            "operator": {
                "match": r"[+\-*/%=<>!&|^~]+",
                "name": "keyword.operator.mxsl"
            },
            "variable": {
                "match": r"[a-zA-Z_$][\w$]*",
                "name": "variable.other.mxsl"
            },
            "paren": {
                "match": r"[{}()\[\]]",
                "name": "punctuation.section.mxsl"
            },
            "block": {
                "begin": r"\{",
                "end": r"\}",
                "name": "meta.block.mxsl",
                "patterns": [{"include": "#expression"}]
            }
        }
    }


def generate_extension_js() -> str:
    """Return the VS Code extension entry point that converts/validates the active file."""
    return r'''// MXSL VS Code extension entry point.
// Two commands backed by the bundled WebAssembly engine:
//   mxsl.convert  - convert the open file between MXSL and MTLX, show result in a new editor
//   mxsl.validate - run the conversion without creating a new file (success/error only)
const vscode = require('vscode');
const path = require('path');
const { pathToFileURL } = require('url');

// Lazily-loaded WebAssembly engine, shared across conversions.
let enginePromise = null;

// Persistent output channel that logs conversion results and errors.
let outputChannel = null;

function activate(context) {
    context.subscriptions.push(
        vscode.commands.registerCommand('mxsl.convert', () => convertActiveFile(context)),
        vscode.commands.registerCommand('mxsl.validate', () => validateActiveFile(context))
    );
}

function deactivate() {}

function getLibDir(context) {
    return path.join(context.extensionUri.fsPath, 'media', 'lib');
}

// Load (once) and return the mxslc WebAssembly engine. The locateFile callback
// must return an absolute filesystem path so the bundled .wasm / .data files
// are found regardless of the process working directory (this is what prevents
// the load from hanging on the data package).
async function loadEngine(context) {
    if (!enginePromise) {
        enginePromise = (async () => {
            const libDir = getLibDir(context);
            const mod = await import(pathToFileURL(path.join(libDir, 'JsMxslc.mjs')).href);
            const factory = mod.default;
            return factory({ locateFile: (file) => path.join(libDir, file) });
        })();
    }
    return enginePromise;
}

function errorMessage(err) {
    return (err && err.message) ? err.message : String(err);
}

// Lazy Output Channel under View -> Output -> MXSL.
function getOutputChannel() {
    if (!outputChannel) {
        outputChannel = vscode.window.createOutputChannel('MXSL');
    }
    return outputChannel;
}

// Append a timestamped line to the output channel.
function log(prefix, ...parts) {
    const line = '[' + new Date().toLocaleTimeString() + '] ' + prefix + ' ' +
        parts.join(' ').replace(/\s+$/g, '');
    getOutputChannel().appendLine(line);
    return line;
}

// Build a full, multi-line error description (name + message + stack).
function errorDetail(err) {
    const name = (err && err.name) ? err.name : 'Error';
    let detail = name + ': ' + errorMessage(err);
    if (err && err.stack) {
        detail += '\n' + err.stack;
    }
    return detail;
}

// Determine the conversion direction for a document, or null if unsupported.
function getKind(doc) {
    const ext = path.extname(doc.fileName).toLowerCase();
    const lang = doc.languageId;
    const isMxsl = lang === 'mxsl' || ext === '.mxsl';
    const isMtlx = lang === 'xml' || ext === '.mtlx' || ext === '.xml';
    if (!isMxsl && !isMtlx) return null;
    return { isMxsl, isMtlx, label: isMxsl ? 'MXSL → MTLX' : 'MTLX → MXSL' };
}

// Resolve the active editor and its conversion kind, warning if not usable.
async function getActiveTarget() {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showWarningMessage('MXSL: open a .mxsl or .mtlx file first.');
        return null;
    }
    const doc = editor.document;
    const kind = getKind(doc);
    if (!kind) {
        vscode.window.showWarningMessage(
            'MXSL: unsupported file type "' + path.extname(doc.fileName).toLowerCase() +
            '". Open a .mxsl or .mtlx file.'
        );
        return null;
    }
    return { editor, doc, kind };
}

// Run the conversion and return the resulting string.
function runConversion(mx, kind, source) {
    if (kind.isMxsl) {
        const opts = new mx.CompileOptions();
        opts.version = '1.39.5';
        opts.reduceGraph = true;
        opts.errorOnMissingGlobals = true;
        opts.errorOnUnusedGlobals = true;
        try {
            return mx.compileSlxToMtlx(source, opts);
        } finally {
            opts.delete();
        }
    }
    return mx.decompileMtlxToSlx(source);
}

// Log an error to the output channel and show a toast with a 'Show Output' action.
async function reportError(prefix, e) {
    const detail = errorDetail(e);
    log('ERROR', detail);
    const action = await vscode.window.showErrorMessage(
        prefix + ': ' + errorMessage(e),
        'Show Output'
    );
    if (action === 'Show Output') {
        getOutputChannel().show();
    }
}

// Convert the open file and show the result in a new untitled editor.
async function convertActiveFile(context) {
    const target = await getActiveTarget();
    if (!target) return;
    const { doc, kind } = target;
    const status = vscode.window.setStatusBarMessage('MXSL: ' + kind.label + ' ...');
    try {
        const mx = await loadEngine(context);
        const result = runConversion(mx, kind, doc.getText());
        const outDoc = await vscode.workspace.openTextDocument({
            content: result,
            language: kind.isMxsl ? 'xml' : 'mxsl'
        });
        await vscode.window.showTextDocument(outDoc, {
            preview: true,
            viewColumn: vscode.ViewColumn.Beside
        });
        log('OK', kind.label, '→', result.length, 'chars');
        vscode.window.showInformationMessage('MXSL: converted ' + kind.label + '.');
    } catch (e) {
        await reportError('MXSL conversion failed', e);
    } finally {
        status.dispose();
    }
}

// Validate the open file: run the conversion but do NOT create a new file.
async function validateActiveFile(context) {
    const target = await getActiveTarget();
    if (!target) return;
    const { doc, kind } = target;
    const status = vscode.window.setStatusBarMessage('MXSL: validating ' + kind.label + ' ...');
    try {
        const mx = await loadEngine(context);
        const result = runConversion(mx, kind, doc.getText());
        log('VALIDATE OK', kind.label, '→', result.length, 'chars');
        vscode.window.showInformationMessage(
            'MXSL: validation passed (' + kind.label + ', ' + result.length + ' chars).'
        );
    } catch (e) {
        await reportError('MXSL validation failed', e);
    } finally {
        status.dispose();
    }
}

module.exports = { activate, deactivate };
'''


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate and/or install a VS Code extension for MXSL syntax highlighting."
    )
    parser.add_argument(
        "-g", "--generate",
        action="store_true",
        help="Generate extension files under tools/vscode_extension/mxsl-vscode/"
    )
    parser.add_argument(
        "-i", "--install",
        action="store_true",
        help="Package as .vsix and install via code --install-extension"
    )
    parser.add_argument(
        "-v", "--vsix",
        action="store_true",
        help="Package the extension as mxsl-language-<version>.vsix"
    )
    parser.add_argument(
        "--mtlx-stdlib",
        type=str,
        default=None,
        help="Path to the MaterialX libraries/ directory (containing stdlib/, bxdf/, etc.). "
             "Defaults to the installed MaterialX package's built-in libraries."
    )
    parser.add_argument(
        "-c", "--converter",
        action="store_true",
        help="Bundle the MXSL<->MTLX WebAssembly converter into the extension: copies "
             "JsMxslc.{js,wasm,data} into media/lib and adds a converter webview command."
    )
    parser.add_argument(
        "--js-build-dir",
        type=str,
        default=None,
        help="Path to the JS build 'bin' directory containing JsMxslc.{js,wasm,data}. "
             "Defaults to mxslc++/javascript/build/mxslc/javascript/bin."
    )
    parser.add_argument(
        "--build-js",
        action="store_true",
        help="Build the JS/WASM module with build_javascript.sh even if the artifacts "
             "already exist. Auto-builds whenever artifacts are missing."
    )
    parser.add_argument(
        "--materialx-root",
        type=str,
        default=None,
        help="Path to a MaterialX source tree (used when building the JS module). "
             "Defaults to the MATERIALX_ROOT environment variable."
    )
    parser.add_argument(
        "--emsdk-location",
        type=str,
        default=None,
        help="Path to the Emscripten SDK (used when building the JS module). "
             "Defaults to <repo>/../emsdk."
    )
    parser.add_argument(
        "-p", "--publish",
        action="store_true",
        help="Package with 'vsce' and publish to the VS Code Marketplace "
             "(requires vsce and a marketplace Personal Access Token)."
    )
    args = parser.parse_args()
    if not args.generate and not args.install and not args.vsix and not args.publish:
        parser.print_usage()
        print("error: at least one of --generate, --install, --vsix, or --publish is required")
        sys.exit(1)
    return args


def main():
    args = parse_args()

    if args.generate or args.converter or not OUT_DIR.exists():
        print("Loading MXSL keywords...")
        kw = load_keywords()
        print(f"  Data types: {len(kw['data_types'])}")
        print(f"  Control:    {len(kw['control'])}")

        print("Loading builtins...")
        builtins = load_builtins()
        print(f"  Builtins:   {len(builtins)}")

        print("Loading MTLX nodedef names...")
        mtlx_stdlib_path = Path(args.mtlx_stdlib) if args.mtlx_stdlib else None
        try:
            functions, mtlx_version = load_mtlx_nodedefs(mtlx_stdlib_path)
            print(f"  Functions for version {mtlx_version}:  {len(functions)}")
        except Exception as e:
            print(f"  Warning: Could not load MTLX nodedefs ({e}), using empty list")
            functions = []

        # Resolve / build the JS/WASM artifacts up front so package.json only
        # advertises the converter command when it can actually be bundled.
        with_converter = False
        if args.converter:
            print("\nBundling MXSL <-> MTLX converter...")
            bin_dir = resolve_js_build(args)
            if bin_dir is None:
                print("  Warning: JsMxslc.{js,wasm,data} not found; converter disabled.")
                print("  Re-run with --build-js (and --materialx-root / --emsdk-location as needed),")
                print("  or build manually: javascript/build_javascript.sh")
            else:
                with_converter = True
                print(f"  Using JS build from {bin_dir}")
                copy_js_artifacts(bin_dir)
                src_dir = OUT_DIR / "src"
                src_dir.mkdir(parents=True, exist_ok=True)
                (src_dir / "extension.js").write_text(generate_extension_js())
                print(f"  -> {src_dir / 'extension.js'}")

        # Create output directory
        syntaxes_dir = OUT_DIR / "syntaxes"
        syntaxes_dir.mkdir(parents=True, exist_ok=True)

        # Write package.json
        pkg = generate_package_json(with_converter=with_converter)
        (OUT_DIR / "package.json").write_text(json.dumps(pkg, indent=2) + "\n")
        print(f"  -> {OUT_DIR / 'package.json'}")

        # Write README.md (shown on the Marketplace) and LICENSE
        (OUT_DIR / "README.md").write_text(generate_readme(with_converter))
        print(f"  -> {OUT_DIR / 'README.md'}")
        write_license()

        # Write language-configuration.json
        lang_cfg = generate_language_configuration()
        (OUT_DIR / "language-configuration.json").write_text(json.dumps(lang_cfg, indent=2) + "\n")
        print(f"  -> {OUT_DIR / 'language-configuration.json'}")

        # Write tmLanguage.json
        grammar = generate_tm_grammar(kw, builtins, functions)
        (syntaxes_dir / "mxsl.tmLanguage.json").write_text(json.dumps(grammar, indent=2) + "\n")
        print(f"  -> {syntaxes_dir / 'mxsl.tmLanguage.json'}")

        # Remove any leftover files from the previous (webview-based) generator layout.
        for stale in [OUT_DIR / "media" / "convert.html", OUT_DIR / "media" / "convert.js"]:
            if stale.exists():
                stale.unlink()
                print(f"  Removed stale file {stale}")

        print("\nGeneration complete.")

    if args.install:
        src = OUT_DIR
        if not src.exists():
            print(f"error: no generated extension found at {src}. Run --generate first.")
            sys.exit(1)
        vsix_path = package_vsix(src)
        print(f"\nPackaged to {vsix_path}")
        result = subprocess.run(["code", "--install-extension", str(vsix_path)],
                                capture_output=True, text=True)
        if result.returncode == 0:
            print("Installed successfully via VS Code.")
            print("Reload VS Code (Cmd/Ctrl+Shift+P -> Developer: Reload Window) for the extension to take effect.")
        else:
            print(f"Install via 'code' command failed. Try manually:")
            print(f"  code --install-extension {vsix_path}")
            if result.stderr:
                print(f"  Error: {result.stderr.strip()}")

    if args.vsix:
        src = OUT_DIR
        if not src.exists():
            print(f"error: no generated extension found at {src}. Run --generate first.")
            sys.exit(1)
        vsix_path = package_vsix(src)
        print(f"\nPackaged to {vsix_path}")
        print(f"Install with: code --install-extension {vsix_path}")

    if args.publish:
        publish_extension()

if __name__ == "__main__":
    main()
