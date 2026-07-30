#!/usr/bin/env python3
"""Generate and/or install a VS Code extension for MXSL syntax highlighting.

Usage:
    python gen_extension.py --generate     # generate extension files
    python gen_extension.py --install      # install to ~/.vscode/extensions/
    python gen_extension.py --generate --install   # both
"""

import argparse
import json
import os
import re
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


# ---------------------------------------------------------------------------
# Keyword extraction
# ---------------------------------------------------------------------------

def load_keywords() -> dict:
    """Load MXSL keywords from Keyword.py (data types vs control)."""
    import importlib.util

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


def generate_package_json() -> dict:
    return {
        "name": "mxsl-language",
        "displayName": "MXSL Language Support",
        "description": "Syntax highlighting for MXSL (MaterialX Shading Language) files",
        "version": "0.1.0",
        "publisher": "shadinglanguagex",
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
    args = parser.parse_args()
    if not args.generate and not args.install and not args.vsix:
        parser.print_usage()
        print("error: at least one of --generate, --install, or --vsix is required")
        sys.exit(1)
    return args


def main():
    args = parse_args()

    if args.generate or args.install or args.vsix:
        if args.generate or not OUT_DIR.exists():
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

            # Create output directory
            syntaxes_dir = OUT_DIR / "syntaxes"
            syntaxes_dir.mkdir(parents=True, exist_ok=True)

            # Write package.json
            pkg = generate_package_json()
            (OUT_DIR / "package.json").write_text(json.dumps(pkg, indent=2) + "\n")
            print(f"  -> {OUT_DIR / 'package.json'}")

            # Write language-configuration.json
            lang_cfg = generate_language_configuration()
            (OUT_DIR / "language-configuration.json").write_text(json.dumps(lang_cfg, indent=2) + "\n")
            print(f"  -> {OUT_DIR / 'language-configuration.json'}")

            # Write tmLanguage.json
            grammar = generate_tm_grammar(kw, builtins, functions)
            (syntaxes_dir / "mxsl.tmLanguage.json").write_text(json.dumps(grammar, indent=2) + "\n")
            print(f"  -> {syntaxes_dir / 'mxsl.tmLanguage.json'}")

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

if __name__ == "__main__":
    main()
