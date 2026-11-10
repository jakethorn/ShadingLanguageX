#!/usr/bin/env python3
"""Generate, package and install the ShadingLanguageX (SLX) extension for VS Code.

The extension's source lives in slx-vscode/. This script generates the parts
that come from the rest of the repository, then builds and packages it.

Usage:
    python gen_extension.py --generate             # regenerate the grammar, language data and version list
    python gen_extension.py --converter            # copy the WebAssembly compiler into the extension
    python gen_extension.py --vsix                 # build and package slx-<version>.vsix
    python gen_extension.py --install              # package and install into VS Code
    python gen_extension.py --publish              # publish to the VS Code Marketplace

Options can be combined, e.g. `--generate --converter --vsix`. Building and
packaging need Node.js and npm.
"""

import argparse
import json
import os
import re
import shutil
import subprocess
import sys
import urllib.error
import urllib.request
import xml.etree.ElementTree as ET
from pathlib import Path

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
REPO_ROOT = Path(__file__).resolve().parent.parent.parent
KEYWORDS_JS = REPO_ROOT / "mxslc++" / "browser_shared" / "keywords.js"
STDLIB_MXSL = REPO_ROOT / "mxslc++" / "libraries" / "stdlib.mxsl"
LIBRARIES_DIR = REPO_ROOT / "mxslc++" / "libraries"

TOOL_DIR = REPO_ROOT / "tools" / "vscode_extension"
EXTENSION_DIR = TOOL_DIR / "slx-vscode"
GRAMMAR_TEMPLATE = TOOL_DIR / "grammar" / "slx.tmLanguage.template.json"
GRAMMAR_OUT = EXTENSION_DIR / "syntaxes" / "slx.tmLanguage.json"
PACKAGE_JSON = EXTENSION_DIR / "package.json"
LIB_DIR = EXTENSION_DIR / "media" / "lib"
DATA_DIR = EXTENSION_DIR / "data"
LANGUAGE_DATA_OUT = DATA_DIR / "language-data.json"

JS_BUILD_BIN = REPO_ROOT / "mxslc++" / "javascript" / "build" / "mxslc" / "javascript" / "bin"
JS_BUILD_SCRIPT = REPO_ROOT / "mxslc++" / "javascript" / "build_javascript.sh"
JS_ARTIFACTS = ["JsMxslc.js", "JsMxslc.wasm", "JsMxslc.data"]

# The MaterialX specification, which node hovers link to. The links go to the
# main branch, so they follow the latest documentation.
SPEC_RAW_URL = "https://raw.githubusercontent.com/AcademySoftwareFoundation/MaterialX/main/documents/Specification/"
SPEC_URL = "https://github.com/AcademySoftwareFoundation/MaterialX/blob/main/documents/Specification/"
SPEC_DOCUMENTS = [
    "MaterialX.StandardNodes.md",
    "MaterialX.PBRSpec.md",
    "MaterialX.NPRSpec.md",
    "MaterialX.Specification.md",
]
# Nodes the specification describes under a heading rather than a node anchor,
# as (pattern matching the node name, heading text, document).
SPEC_HEADINGS = [
    (r"standard_surface", "Autodesk Standard Surface", "MaterialX.PBRSpec.md"),
    (r"UsdPreviewSurface", "UsdPreviewSurface", "MaterialX.PBRSpec.md"),
    (r"gltf_pbr", "Khronos glTF PBR", "MaterialX.PBRSpec.md"),
    (r"open_pbr_surface", "OpenPBR Surface", "MaterialX.PBRSpec.md"),
    (r"disney_principled", "Disney Principled BSDF", "MaterialX.PBRSpec.md"),
    (r"Lama\w+", "MaterialX Lama", "MaterialX.PBRSpec.md"),
    (r"\w+_to_\w+", "Shading Translation Graphs", "MaterialX.PBRSpec.md"),
]

# ---------------------------------------------------------------------------
# Keyword classes. Every keyword in keywords.js that is not listed here is
# highlighted as keyword.other.
# ---------------------------------------------------------------------------
CONTROL_KEYWORDS = {"if", "else", "for", "from", "to", "return", "break", "switch", "case", "default"}
MODIFIER_KEYWORDS = {
    "const", "mutable", "consteval", "comptime", "global", "geomprop", "nodegraph", "nodedef",
    "inline", "uniform", "varying", "ref", "out",
}
# Keywords with rules of their own in the grammar template.
SPECIAL_KEYWORDS = {"class", "this", "null"}
LANGUAGE_CONSTANTS = {"true", "false", "null"}
EXTRA_TYPES = {"void", "auto", "T"}


# ---------------------------------------------------------------------------
# Colours in VS Code's default themes, matching the SLX code editor of the
# MaterialX Playground (MaterialXPlayground js/shared/theme-tokens.js, the
# code-syntax-* tokens, and js/graph/code-view.jsx). Other themes choose their
# own colours.
# ---------------------------------------------------------------------------
PLAYGROUND_COLORS = {
    "dark": {"comment": "#6a9955", "string": "#ce9178", "number": "#b5cea8", "keyword": "#569cd6",
             "type": "#4ec9b0", "directive": "#c586c0", "function": "#dcdcaa",
             # Not from the Playground, which leaves these plain: argument names, as in
             # standard_surface(base_color = c), calls of standard library functions,
             # and macros.
             "argument": "#e49a9a", "builtin": "#cabe91", "macro": "#c3a6e0"},
    "light": {"comment": "#008000", "string": "#a31515", "number": "#08794f", "keyword": "#0000ff",
              "type": "#217a93", "directive": "#af00db", "function": "#795e26",
              "argument": "#ba3b5b", "builtin": "#7e6f44", "macro": "#7b3f9e"},
}
# The default themes, by their ids in current VS Code and in older versions,
# with each one's text colour: the Playground leaves names, calls, operators
# and punctuation as plain text, so those get it.
DEFAULT_THEMES = {
    "dark": [(["Dark 2026"], "#bbbebf"), (["Dark Modern", "Default Dark Modern"], "#cccccc"), (["Dark+", "Default Dark+"], "#d4d4d4")],
    "light": [(["Light 2026"], "#202020"), (["Light Modern", "Default Light Modern"], "#3b3b3b"), (["Light+", "Default Light+"], "#000000")],
}
# Which colour each scope of the grammar gets, by scope prefix; the first
# match wins. A scope that matches none stops generation, so new scopes
# can't go uncoloured.
SCOPE_COLORS = [
    ("comment.", "comment"), ("punctuation.definition.comment.", "comment"),
    ("string.", "string"), ("punctuation.definition.string.", "string"),
    ("constant.numeric.", "number"), ("constant.language.", "number"),
    ("keyword.control.directive.", "directive"), ("punctuation.definition.directive.", "directive"),
    ("keyword.control.", "keyword"), ("storage.modifier.", "keyword"), ("storage.type.class.", "keyword"),
    ("variable.language.this.", "keyword"), ("keyword.other.", "keyword"),
    ("storage.type.", "type"),
    # Attributes, as in @doc, @in.doc and @@colorspace, all in the function colour.
    ("meta.attribute.", "function"), ("entity.other.attribute-name.", "function"),
    ("punctuation.definition.attribute.", "function"),
    ("variable.parameter.named-argument.", "argument"), ("support.function.", "builtin"),
    # Macro names in #define and #ifdef lines, and the predefined macros. Macros used elsewhere are
    # found by the extension's semantic highlighting, which package.json maps to the first scope.
    ("entity.name.function.preprocessor.", "macro"), ("support.constant.macro.", "macro"),
    ("entity.name.", "plain"), ("support.", "plain"), ("variable.", "plain"), ("keyword.operator.", "plain"),
    ("punctuation.", "plain"), ("meta.", "plain"),
]


def collect_scopes(node) -> set[str]:
    """Return every scope name in a grammar."""
    scopes: set[str] = set()
    if isinstance(node, dict):
        for key, value in node.items():
            if key in ("name", "contentName") and isinstance(value, str):
                scopes.add(value)
            else:
                scopes |= collect_scopes(value)
    elif isinstance(node, list):
        for item in node:
            scopes |= collect_scopes(item)
    return scopes


def generate_theme_colors(grammar: dict) -> dict:
    """Return the editor.tokenColorCustomizations for the default themes.

    Rules name full scopes, like keyword.control.slx, so they are more specific
    than the themes' own rules and win over them."""
    by_color: dict[str, list[str]] = {}
    # The grammar's top-level "name" is its display name, not a scope.
    for scope in sorted(collect_scopes([grammar["patterns"], grammar["repository"]])):
        color = next((c for prefix, c in SCOPE_COLORS if scope.startswith(prefix)), None)
        if color is None:
            sys.exit(f"error: no colour for the grammar scope {scope}; add it to SCOPE_COLORS in {Path(__file__).name}")
        by_color.setdefault(color, []).append(scope)
    # The "." and port name in an attribute, as in @in.doc, have scopes used elsewhere too, so colour them by their parent.
    attribute_scopes = collect_scopes(grammar["repository"]["attributes"]["captures"])
    by_color["function"] += [f"meta.attribute.slx {s}" for s in sorted(attribute_scopes) if s in by_color["plain"]]

    def rules(colors: dict[str, str]) -> list[dict]:
        return [
            {"name": f"SLX {color}", "scope": by_color[color], "settings": {"foreground": colors[color], "fontStyle": ""}}
            for color in sorted(colors)
            if color in by_color
        ]

    customizations: dict[str, dict] = {}
    for mode, themes in DEFAULT_THEMES.items():
        all_ids = [theme_id for ids, _ in themes for theme_id in ids]
        customizations["".join(f"[{i}]" for i in all_ids)] = {"textMateRules": rules(PLAYGROUND_COLORS[mode])}
        for ids, text_color in themes:
            customizations["".join(f"[{i}]" for i in ids)] = {"textMateRules": rules({"plain": text_color})}
    return customizations


# ---------------------------------------------------------------------------
# Language data
# ---------------------------------------------------------------------------

def load_keywords() -> tuple[list[str], list[str]]:
    """Return the control keywords and data types from browser_shared/keywords.js,
    which mirrors the mxslc++ TokenType keyword enum (include/TokenType.h)."""
    text = KEYWORDS_JS.read_text(encoding="utf-8")

    def extract_array(name: str) -> list[str]:
        match = re.search(rf"\b{name}\s*:\s*\[(.*?)\]", text, re.S)
        return re.findall(r"'([^']*)'", match.group(1)) if match else []

    return extract_array("control"), extract_array("dataTypes")


def load_stdlib() -> tuple[dict[str, str], set[str]]:
    """Return the type aliases (alias -> type) and function names defined in stdlib.mxsl."""
    text = STDLIB_MXSL.read_text(encoding="utf-8")
    aliases = dict(re.findall(r"^\s*using\s+(\w+)\s*=\s*(\w+)\s*;", text, re.M))
    functions = set(re.findall(r"\b([A-Za-z_]\w*)\s*(?:<[^<>()]*>)?\s*\(", text))
    return aliases, {f for f in functions if not f.startswith("__")}


def version_key(version: str) -> tuple[int, ...]:
    return tuple(int(part) for part in version.split(".") if part.isdigit())


def find_materialx_versions() -> list[str]:
    """Return the MaterialX versions the compiler ships libraries for, oldest first."""
    versions = [d.name for d in LIBRARIES_DIR.iterdir() if d.is_dir() and re.fullmatch(r"\d+(\.\d+)+", d.name)]
    return sorted(versions, key=version_key)


def load_library_documents(version: str) -> list[ET.Element]:
    """Return the root elements of the MaterialX library files for a version,
    the same files the compiler loads."""
    roots = []
    for path in sorted((LIBRARIES_DIR / version).rglob("*.mtlx")):
        # Shader implementation files hold no definitions.
        if any(part.startswith("gen") for part in path.relative_to(LIBRARIES_DIR / version).parts[:-1]):
            continue
        try:
            roots.append(ET.parse(path).getroot())
        except ET.ParseError as error:
            print(f"  Warning: skipping {path}: {error}")
    return roots


# Docs are not taken from the libraries: hovers link to the specification instead.
INPUT_ATTRIBUTES = ["value", "defaultgeomprop", "enum", "uiname"]


def describe_input(element: ET.Element) -> dict:
    described = {"name": element.get("name"), "type": element.get("type")}
    for attribute in INPUT_ATTRIBUTES:
        if element.get(attribute) is not None:
            described[attribute] = element.get(attribute)
    if element.get("uniform") == "true":
        described["uniform"] = True
    return described


def collect_ports(nodedef: ET.Element, nodedefs: dict[str, ET.Element], tag: str, depth: int = 0) -> list[dict]:
    """Return a nodedef's inputs or outputs, including those it inherits, in MaterialX order."""
    ports: dict[str, dict] = {}
    parent = nodedefs.get(nodedef.get("inherit", ""))
    if parent is not None and depth < 16:
        ports.update((p["name"], p) for p in collect_ports(parent, nodedefs, tag, depth + 1))
    for element in nodedef.findall(tag):
        ports[element.get("name")] = (
            describe_input(element) if tag == "input" else {"name": element.get("name"), "type": element.get("type")}
        )
    return list(ports.values())


def load_node_signatures(versions: list[str]) -> dict[str, list[dict]]:
    """Return the signatures of every MaterialX node, by node name. A signature
    that is the same in several versions is listed once, with those versions."""
    signatures: dict[str, dict] = {}
    for version in versions:
        nodedefs = {nd.get("name"): nd for root in load_library_documents(version) for nd in root.iter("nodedef")}
        for name, nodedef in nodedefs.items():
            # The compiler only loads the default version of a versioned nodedef.
            if nodedef.get("version") and nodedef.get("isdefaultversion") != "true":
                continue
            signature = {
                "node": nodedef.get("node"),
                "nodedef": name,
                "group": nodedef.get("nodegroup"),
                "inputs": collect_ports(nodedef, nodedefs, "input"),
                "outputs": collect_ports(nodedef, nodedefs, "output"),
            }
            signature = {k: v for k, v in signature.items() if v is not None}
            key = json.dumps(signature, sort_keys=True)
            signatures.setdefault(key, {**signature, "versions": []})["versions"].append(version)

    nodes: dict[str, list[dict]] = {}
    for signature in signatures.values():
        nodes.setdefault(signature.pop("node"), []).append(signature)
    return dict(sorted(nodes.items()))


def to_github_heading_anchor(heading: str) -> str:
    """Return the anchor GitHub gives a Markdown heading."""
    return re.sub(r"[^\w\- ]", "", heading.strip().lower()).replace(" ", "-")


def load_spec_links(node_names: list[str]) -> dict[str, str]:
    """Return the URL of each node's section in the MaterialX specification on
    GitHub, for the nodes it describes. The specification is downloaded so the
    links match its current anchors and headings; if it can't be, there are no
    links."""
    anchors: dict[str, str] = {}
    headings: dict[tuple[str, str], str] = {}
    for document in SPEC_DOCUMENTS:
        try:
            with urllib.request.urlopen(SPEC_RAW_URL + document, timeout=30) as response:
                text = response.read().decode("utf-8")
        except (urllib.error.URLError, TimeoutError) as error:
            print(f"  Warning: couldn't download {document} ({error}); node hovers won't link to the specification.")
            return {}
        for anchor in re.findall(r'<a id="(node-[^"]+)"', text):
            anchors.setdefault(anchor, document)
        for heading in re.findall(r"^#+\s+(.+?)\s*$", text, re.M):
            headings[(document, heading)] = to_github_heading_anchor(heading)

    links: dict[str, str] = {}
    for name in node_names:
        anchor = "node-" + name.lower().replace("_", "-")
        if anchor in anchors:
            links[name] = f"{SPEC_URL}{anchors[anchor]}#{anchor}"
            continue
        for pattern, heading, document in SPEC_HEADINGS:
            if re.fullmatch(pattern, name) and (document, heading) in headings:
                links[name] = f"{SPEC_URL}{document}#{headings[(document, heading)]}"
                break
    return links


def load_materialx_types(versions: list[str]) -> set[str]:
    types = {
        typedef.get("name")
        for version in versions
        for root in load_library_documents(version)
        for typedef in root.iter("typedef")
    }
    return {t for t in types if t and t != "none" and not t.endswith("array")}


def load_language(versions: list[str]) -> dict:
    """Return the keywords, types and functions of SLX, for the grammar and the
    extension's completion, hover and parameter hints."""
    keywords, data_types = load_keywords()
    aliases, stdlib_functions = load_stdlib()
    nodes = load_node_signatures(versions)
    all_keywords = set(keywords) | LANGUAGE_CONSTANTS
    types = set(data_types) | set(aliases) | load_materialx_types(versions) | EXTRA_TYPES
    return {
        "versions": versions,
        "keywords": {
            "control": sorted(CONTROL_KEYWORDS),
            "modifiers": sorted(MODIFIER_KEYWORDS),
            "constants": sorted(LANGUAGE_CONSTANTS),
            "other": sorted(set(keywords) - CONTROL_KEYWORDS - MODIFIER_KEYWORDS - LANGUAGE_CONSTANTS),
            "all": sorted(all_keywords),
        },
        "types": sorted(types),
        "aliases": dict(sorted(aliases.items())),
        "builtinFunctions": sorted((set(nodes) | stdlib_functions) - all_keywords - types),
        "nodes": nodes,
        "specLinks": load_spec_links(list(nodes)),
    }


def write_language_data(language: dict) -> None:
    """Write the language data and a copy of stdlib.mxsl into data/."""
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    LANGUAGE_DATA_OUT.write_text(json.dumps(language, separators=(",", ":")) + "\n", encoding="utf-8")
    shutil.copy2(STDLIB_MXSL, DATA_DIR / "stdlib.mxsl")
    signatures = sum(len(s) for s in language["nodes"].values())
    print(f"  Nodes:             {len(language['nodes'])} ({signatures} signatures)")
    print(f"  Spec links:        {len(language['specLinks'])}")
    print(f"  -> {LANGUAGE_DATA_OUT} ({LANGUAGE_DATA_OUT.stat().st_size // 1024} KB)")
    print(f"  -> {DATA_DIR / 'stdlib.mxsl'}")


def join_words(words: set[str]) -> str:
    """Return a regex alternation of the words, longest first so prefixes don't win."""
    return "|".join(re.escape(w) for w in sorted(words, key=lambda w: (-len(w), w)))


def generate_grammar(language: dict) -> dict:
    words = {
        "KEYWORDS": set(language["keywords"]["all"]),
        "CONTROL_KEYWORDS": CONTROL_KEYWORDS,
        "MODIFIER_KEYWORDS": MODIFIER_KEYWORDS,
        "OTHER_KEYWORDS": set(language["keywords"]["other"]) - SPECIAL_KEYWORDS,
        "TYPES": set(language["types"]),
        "BUILTIN_FUNCTIONS": set(language["builtinFunctions"]),
    }
    print(f"  Keywords:          {len(words['KEYWORDS'])}")
    print(f"  Types:             {len(words['TYPES'])}")
    print(f"  Built-in functions: {len(words['BUILTIN_FUNCTIONS'])} (MaterialX {', '.join(language['versions'])})")

    grammar = json.loads(GRAMMAR_TEMPLATE.read_text(encoding="utf-8"))
    grammar.pop("$comment", None)
    text = json.dumps(grammar, indent=2)
    for name, values in words.items():
        # The words are inserted into JSON strings, so their backslashes need escaping.
        text = text.replace("{{" + name + "}}", json.dumps(join_words(values))[1:-1])
    leftover = re.findall(r"\{\{\w+\}\}", text)
    if leftover:
        sys.exit(f"error: unknown placeholders in {GRAMMAR_TEMPLATE}: {', '.join(sorted(set(leftover)))}")

    GRAMMAR_OUT.parent.mkdir(parents=True, exist_ok=True)
    GRAMMAR_OUT.write_text(text + "\n", encoding="utf-8")
    print(f"  -> {GRAMMAR_OUT}")
    return json.loads(text)


def update_package_json(versions: list[str], theme_colors: dict) -> None:
    """List the supported MaterialX versions in the slx.materialxVersion setting,
    and set the colours of the default themes."""
    package = json.loads(PACKAGE_JSON.read_text(encoding="utf-8"))
    before = json.dumps(package)
    setting = package["contributes"]["configuration"]["properties"]["slx.materialxVersion"]
    setting["enum"] = versions
    setting["default"] = versions[-1]
    package["contributes"]["configurationDefaults"] = {"editor.tokenColorCustomizations": theme_colors}
    if json.dumps(package) == before:
        return
    PACKAGE_JSON.write_text(json.dumps(package, indent=2) + "\n", encoding="utf-8")
    print(f"  -> {PACKAGE_JSON} (MaterialX versions {', '.join(versions)}, default theme colours)")


def copy_license() -> None:
    shutil.copy2(REPO_ROOT / "LICENSE", EXTENSION_DIR / "LICENSE")


# ---------------------------------------------------------------------------
# WebAssembly compiler
# ---------------------------------------------------------------------------

def run_js_build(args: argparse.Namespace) -> None:
    """Build the JsMxslc WebAssembly module with javascript/build_javascript.sh."""
    emsdk = args.emsdk_location or str(REPO_ROOT.parent / "emsdk")
    materialx_root = args.materialx_root or os.environ.get("MATERIALX_ROOT", "")
    command = ["bash", str(JS_BUILD_SCRIPT), emsdk] + ([materialx_root] if materialx_root else [])
    print("  Building with:", " ".join(command))
    result = subprocess.run(command, cwd=REPO_ROOT, check=False)
    if result.returncode != 0:
        print(f"  Warning: build_javascript.sh exited with code {result.returncode}")


def copy_converter(args: argparse.Namespace) -> bool:
    """Copy the JsMxslc files into media/lib, building them first if they are
    missing or --build-js is given. Returns False if they could not be found."""
    bin_dir = Path(args.js_build_dir) if args.js_build_dir else JS_BUILD_BIN
    if args.build_js or not all((bin_dir / name).exists() for name in JS_ARTIFACTS):
        run_js_build(args)
    if not all((bin_dir / name).exists() for name in JS_ARTIFACTS):
        print(f"  Warning: JsMxslc.{{js,wasm,data}} not found in {bin_dir}.")
        print("  Re-run with --build-js (and --materialx-root / --emsdk-location as needed).")
        return False

    LIB_DIR.mkdir(parents=True, exist_ok=True)
    # The engine worker imports the module as an ES module, which Node only
    # does for a .mjs file.
    shutil.copy2(bin_dir / "JsMxslc.js", LIB_DIR / "JsMxslc.mjs")
    shutil.copy2(bin_dir / "JsMxslc.wasm", LIB_DIR / "JsMxslc.wasm")
    shutil.copy2(bin_dir / "JsMxslc.data", LIB_DIR / "JsMxslc.data")
    print(f"  -> {LIB_DIR} (from {bin_dir})")
    return True


# ---------------------------------------------------------------------------
# Building and packaging
# ---------------------------------------------------------------------------

def run_npm(*npm_args: str) -> None:
    npm = shutil.which("npm")
    if npm is None:
        sys.exit("error: npm was not found. Install Node.js (which includes npm) and retry.")
    print(f"  npm {' '.join(npm_args)}")
    result = subprocess.run([npm, *npm_args], cwd=EXTENSION_DIR, check=False)
    if result.returncode != 0:
        sys.exit(f"error: 'npm {' '.join(npm_args)}' failed with exit code {result.returncode}")


def install_dependencies() -> None:
    if not (EXTENSION_DIR / "node_modules").exists():
        run_npm("ci")


def package_vsix() -> Path:
    """Build the extension and package it with vsce, returning the .vsix path."""
    install_dependencies()
    if not (LIB_DIR / "JsMxslc.mjs").exists():
        print("  Warning: the WebAssembly compiler is not in media/lib; run with --converter to bundle it.")
        print("           Without it, the extension only provides highlighting and snippets.")
    run_npm("run", "package")
    version = json.loads(PACKAGE_JSON.read_text(encoding="utf-8"))["version"]
    vsix = TOOL_DIR / f"slx-{version}.vsix"
    print(f"\nPackaged to {vsix}")
    return vsix


def install_vsix(vsix: Path) -> None:
    code = shutil.which("code")
    if code is None:
        print(f"'code' was not found on the PATH. Install manually with:\n  code --install-extension {vsix}")
        return
    result = subprocess.run([code, "--install-extension", str(vsix)], capture_output=True, text=True)
    if result.returncode == 0:
        print("Installed. Reload VS Code (Developer: Reload Window) for the extension to take effect.")
    else:
        print(f"Install failed. Try manually:\n  code --install-extension {vsix}")
        if result.stderr:
            print(f"  Error: {result.stderr.strip()}")


def publish_extension() -> None:
    """Publish to the VS Code Marketplace. Needs a VSCE_PAT for the publisher."""
    install_dependencies()
    run_npm("exec", "--", "vsce", "publish", "--no-dependencies")
    print("\nPublished.")


# ---------------------------------------------------------------------------
# Command line
# ---------------------------------------------------------------------------

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate, package and install the SLX extension for VS Code.")
    parser.add_argument("-g", "--generate", action="store_true",
                        help="Regenerate syntaxes/slx.tmLanguage.json, data/ and the MaterialX version list in package.json.")
    parser.add_argument("-c", "--converter", action="store_true",
                        help="Copy the WebAssembly compiler (JsMxslc.{js,wasm,data}) into media/lib, "
                             "building it first if it is missing.")
    parser.add_argument("-v", "--vsix", action="store_true",
                        help="Build the extension and package it as slx-<version>.vsix.")
    parser.add_argument("-i", "--install", action="store_true",
                        help="Package the extension and install it with 'code --install-extension'.")
    parser.add_argument("-p", "--publish", action="store_true",
                        help="Publish to the VS Code Marketplace (needs a VSCE_PAT for the publisher).")
    parser.add_argument("--js-build-dir", default=None,
                        help="Folder holding JsMxslc.{js,wasm,data}. "
                             "Defaults to mxslc++/javascript/build/mxslc/javascript/bin.")
    parser.add_argument("--build-js", action="store_true",
                        help="Rebuild the WebAssembly compiler even if it already exists.")
    parser.add_argument("--materialx-root", default=None,
                        help="MaterialX source tree, for building the WebAssembly compiler. "
                             "Defaults to the MATERIALX_ROOT environment variable.")
    parser.add_argument("--emsdk-location", default=None,
                        help="Emscripten SDK, for building the WebAssembly compiler. Defaults to <repo>/../emsdk.")
    args = parser.parse_args()
    if not (args.generate or args.converter or args.vsix or args.install or args.publish):
        parser.print_usage()
        sys.exit("error: give at least one of --generate, --converter, --vsix, --install or --publish")
    return args


def main() -> None:
    args = parse_args()

    if args.generate:
        print("Generating the grammar, language data and version list...")
        versions = find_materialx_versions()
        language = load_language(versions)
        grammar = generate_grammar(language)
        write_language_data(language)
        update_package_json(versions, generate_theme_colors(grammar))
        copy_license()

    if args.converter:
        print("\nBundling the WebAssembly compiler...")
        copy_converter(args)

    if args.vsix or args.install:
        print("\nPackaging...")
        vsix = package_vsix()
        if args.install:
            install_vsix(vsix)

    if args.publish:
        print("\nPublishing...")
        publish_extension()


if __name__ == "__main__":
    main()
