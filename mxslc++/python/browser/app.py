"""Flask web app for converting between MTLX (MaterialX) and MXSL formats."""

import functools

from flask import Flask, jsonify, render_template, request

app = Flask(__name__)

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def _import_mxslc() -> object:
    """Lazy-import mxslc so the app can at least start without it."""
    import mxslc as _m

    return _m


@functools.cache
def get_keywords() -> dict:
    """Return the keywords, type names and function names, from the compiler."""
    mxslc = _import_mxslc()
    reflection = mxslc.reflect()
    return {
        "keywords": mxslc.get_keywords(),
        "types": [t.name for t in reflection.types],
        "functions": [f.name for f in reflection.functions],
    }


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
        options.default_function_behaviour = getattr(
            mxslc.FunctionBehaviour, opts.get("default_function_behaviour", "NodeDef"))
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
    """Return the keywords, type names and function names, for highlighting."""
    try:
        return jsonify(get_keywords())
    except Exception as exc:
        return jsonify({"error": str(exc)}), 500


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
