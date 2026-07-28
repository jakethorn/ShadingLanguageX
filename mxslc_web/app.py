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


def _load_keywords() -> list[str]:
    """Load MXSL keywords from the mxslc Python source Keyword enum."""
    try:
        # Path to Keyword.py relative to this file
        kw_path = Path(__file__).resolve().parent.parent / "mxslc" / "mxslc" / "Keyword.py"
        if not kw_path.exists():
            kw_path = Path(__file__).resolve().parent / ".." / "mxslc" / "mxslc" / "Keyword.py"
        if not kw_path.exists():
            return []
        spec = importlib.util.spec_from_file_location("Keyword", str(kw_path))
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        return sorted({str(k) for k in mod.Keyword})
    except Exception:
        return []


def _load_builtins() -> list[str]:
    """Load MXSL built-in function names from the standard library file."""
    try:
        lib_path = Path(__file__).resolve().parent.parent / "mxslc++" / "libraries" / "stdlib.mxsl"
        if not lib_path.exists():
            lib_path = Path(__file__).resolve().parent / ".." / "mxslc++" / "libraries" / "stdlib.mxsl"
        if not lib_path.exists():
            return []
        import re
        with open(lib_path) as f:
            content = f.read()
        funcs = set(re.findall(r'\b([a-zA-Z_][a-zA-Z0-9_]*)\s*\(', content))
        # Filter out internal helpers (__xxx__) and common C names
        funcs = {f for f in funcs if not f.startswith("__")}
        return sorted(funcs)
    except Exception:
        return []


_KEYWORDS: list[str] | None = None
_BUILTINS: list[str] | None = None


def get_keywords() -> list[str]:
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
# Routes
# ---------------------------------------------------------------------------

@app.route("/")
def index():
    """Serve the main page."""
    return render_template("index.html")


@app.route("/api/compile", methods=["POST"])
def api_compile():
    """Convert MXSL source → MTLX XML."""
    data = request.get_json(force=True)
    source: str = data.get("source", "").strip()

    if not source:
        return jsonify({"error": "No MXSL source provided."}), 400

    try:
        mxslc = _import_mxslc()
        result = mxslc.compile_string_to_string(source)
        return jsonify({"result": result})
    except Exception as exc:
        return jsonify({"error": str(exc)}), 400


@app.route("/api/decompile", methods=["POST"])
def api_decompile():
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


@app.route("/api/keywords")
def api_keywords():
    """Return MXSL keywords and built-in function names."""
    return jsonify({
        "keywords": get_keywords(),
        "builtins": get_builtins()
    })


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    app.run(debug=True, host="0.0.0.0", port=5001)
