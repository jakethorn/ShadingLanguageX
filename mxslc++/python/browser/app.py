"""Flask web app for converting between MTLX (MaterialX) and MXSL formats."""

import importlib.util
import os
from pathlib import Path

from flask import Flask, jsonify, render_template, request

app = Flask(__name__)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _import_mxslc() -> object:
    """Lazy-import mxslc so the app can at least start without it."""
    import mxslc as _m

    return _m

def _get_mtlx_definition_names() -> list[str]:
    """Get the names of all definitions in the installed MaterialX package."""
    import MaterialX as mx
    stdlib = mx.createDocument()
    searchPath = mx.getDefaultDataSearchPath()
    libraryFolders = []
    libraryFolders.extend(mx.getDefaultDataLibraryFolders())
    mx.loadLibraries(libraryFolders, searchPath, stdlib)
    defs = stdlib.getNodeDefs()
    unique_names = set(nd.getNodeString() for nd in defs)
    return sorted(unique_names)

def _load_keywords() -> dict:
    mtlx_keywords = _get_mtlx_definition_names()
    print(f"Found {len(mtlx_keywords)} MTLX keywords")

    """Load MXSL keywords from the mxslc Python source Keyword enum, categorized."""
    try:
        kw_path = Path(__file__).resolve().parent.parent.parent.parent
        kw_path = kw_path / "mxslc" / "mxslc" / "Keyword.py"
        if not kw_path.exists():
            print('Could not find keywords path', kw_path)
            return {"data_types": [], "control": [], "all": []}
        else:
            print('Found keywords path', kw_path)
        spec = importlib.util.spec_from_file_location("Keyword", str(kw_path))
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)

        data_types = sorted({str(k) for k in mod.Keyword.DATA_TYPES()})
        all_kw = sorted({str(k) for k in mod.Keyword})
        control = sorted(set(all_kw) - set(data_types))

        # MTLX nodedef names become function names for highlighting.
        all_kw.extend(mtlx_keywords)
        return {"data_types": data_types, "control": control, "functions": mtlx_keywords, "all": all_kw}
    except Exception:
        return {"data_types": [], "control": [], "all": []}


def _load_builtins() -> list[str]:
    """Load MXSL built-in function names from the installed package's stdlib."""
    try:
        import mxslc as _m
        lib_path = Path(_m.__file__).parent / "libraries" / "stdlib.mxsl"
        if not lib_path.exists():
            return []
        import re
        with open(lib_path) as f:
            content = f.read()
        funcs = set(re.findall(r'\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(', content))
        funcs = {f for f in funcs if not f.startswith("__")}
        return sorted(funcs)
    except Exception:
        return []


_KEYWORDS: dict | None = None
_BUILTINS: list[str] | None = None


def get_keywords() -> dict:
    global _KEYWORDS
    if _KEYWORDS is None:
        _KEYWORDS = _load_keywords()
    return _KEYWORDS


def get_builtins() -> list[str]:
    global _BUILTINS
    if _BUILTINS is None:
        _BUILTINS = _load_builtins()
    return _BUILTINS


# ---------------------------------------------------------------------------
# Handlers
# ---------------------------------------------------------------------------

def _index():
    """Serve the main page."""
    return render_template("index.html")


def _api_compile():
    """Convert MXSL source → MTLX XML."""
    data = request.get_json(force=True)
    source: str = data.get("source", "").strip()
    opts: dict = data.get("options", {})

    if not source:
        return jsonify({"error": "No MXSL source provided."}), 400

    try:
        mxslc = _import_mxslc()
        options = mxslc.CompileOptions()
        options.version = opts.get("version", "1.39.5")
        options.reduce_graph = opts.get("reduce_graph", True)
        options.error_on_missing_globals = opts.get("error_on_missing_globals", True)
        options.error_on_unused_globals = opts.get("error_on_unused_globals", True)
        options.single_use_as_nodegraph = opts.get("single_use_as_nodegraph", True)
        result = mxslc.compile_string_to_string(source, options)
        return jsonify({"result": result})
    except Exception as exc:
        return jsonify({"error": str(exc)}), 400


def _api_decompile():
    """Convert MTLX XML → MXSL source."""
    data = request.get_json(force=True)
    source: str = data.get("source", "").strip()

    if not source:
        return jsonify({"error": "No MTLX source provided."}), 400

    try:
        mxslc = _import_mxslc()
        result = mxslc.decompile_string_to_string(source)
        return jsonify({"result": result})
    except Exception as exc:
        return jsonify({"error": str(exc)}), 400


def _api_keywords():
    """Return MXSL keywords (categorized) and built-in function names."""
    kw = get_keywords()
    return jsonify({
        "data_types": kw.get("data_types", []),
        "control": kw.get("control", []),
        "functions": kw.get("functions", []),
        "all": kw.get("all", []),
        "builtins": get_builtins()
    })


# ---------------------------------------------------------------------------
# App factory
# ---------------------------------------------------------------------------

def create_app() -> Flask:
    _app = Flask(__name__)

    _app.add_url_rule("/",             endpoint="index",     view_func=_index)
    _app.add_url_rule("/api/compile",  endpoint="api_compile",  view_func=_api_compile,  methods=["POST"])
    _app.add_url_rule("/api/decompile", endpoint="api_decompile", view_func=_api_decompile, methods=["POST"])
    _app.add_url_rule("/api/keywords", endpoint="api_keywords", view_func=_api_keywords)

    return _app


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------



app = create_app()

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5001)
