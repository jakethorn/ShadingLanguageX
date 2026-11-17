# Sphinx configuration for the ShadingLanguageX site: the site's pages in this
# folder, the Markdown docs in docs/, the Hello SLX tutorial, the C++/Python/
# JavaScript API references and the browser compiler.
#
# Build with build.py (from the repository root), which stages the pages for
# Sphinx and passes this folder as the configuration directory:
#     pip install -r docs/site/requirements.txt    # and Doxygen, e.g. apt install doxygen
#     python docs/site/build.py
#
# The API references are generated, not written:
#   * C++:        Doxygen reads the public headers listed in Doxyfile.
#   * Python:     sphinx-autoapi reads mxslc++/python/mxslc/_mxslc.pyi, the stubs
#                 made by mxslc++/python/generate_stubs.sh.
#   * JavaScript: JsMxslc.d.ts, emitted by the JavaScript build
#                 (mxslc++/javascript/build_javascript.sh).

import os
import re
import shutil
import subprocess
import tomllib
from pathlib import Path

from sphinx.util import logging

logger = logging.getLogger(__name__)

SITE = Path(__file__).resolve().parent
ROOT = SITE.parents[1]
MXSLC = ROOT / "mxslc++"
JS_DTS = MXSLC / "javascript" / "build" / "mxslc" / "javascript" / "bin" / "JsMxslc.d.ts"
COMPILER = MXSLC / "javascript" / "browser"
REPOSITORY = "https://github.com/jakethorn/ShadingLanguageX"

project = "ShadingLanguageX"
author = "Jacob Thorn"
copyright = "Jacob Thorn"
release = tomllib.loads((MXSLC / "pyproject.toml").read_text())["project"]["version"]
version = release

extensions = [
    "myst_parser",
    "breathe",
    "autoapi.extension",
]

exclude_patterns = ["docs/Proposals.md"]

# The docs are written for GitHub, which allows skipped heading levels, e.g., # then ###.
suppress_warnings = ["myst.header"]

# Unlabelled code blocks are SLX, which Pygments doesn't know.
highlight_language = "none"

html_theme = "furo"
html_title = f"SLX {release}"
html_theme_options = {
    "source_repository": f"{REPOSITORY}/",
    "source_branch": "main",
}

# -- Markdown ----------------------------------------------------------------

# Anchors for every heading level, as GitHub makes, so links such as
# APIDocumentation.md#reflection keep working.
myst_heading_anchors = 6

# -- C++ ---------------------------------------------------------------------

DOXYGEN_XML = SITE / "_build" / "doxygen" / "xml"

if shutil.which("doxygen") is None:
    raise RuntimeError("Doxygen is required to build the C++ API reference, e.g. 'apt install doxygen'.")
DOXYGEN_XML.parent.mkdir(parents=True, exist_ok=True)
subprocess.run(["doxygen", "Doxyfile"], cwd=SITE, check=True)

breathe_projects = {"mxslc": str(DOXYGEN_XML)}
breathe_default_project = "mxslc"
breathe_default_members = ("members", "undoc-members")

# -- Python ------------------------------------------------------------------

autoapi_dirs = [str(MXSLC / "python" / "mxslc")]
autoapi_file_patterns = ["*.pyi", "*.py"]
autoapi_ignore = ["*__main__*"]
autoapi_options = ["members", "undoc-members", "imported-members", "show-inheritance", "show-module-summary"]
autoapi_root = "api/python"
autoapi_template_dir = str(SITE / "_templates" / "autoapi")

# -- Source fixes ------------------------------------------------------------

FENCE = re.compile(r"^\s*(```|~~~)")
HEADING = re.compile(r"^#{1,6}\s")
HTML_TITLE = re.compile(r"^<h1[^>]*>(.*?)</h1>[ \t]*$", re.MULTILINE)
GITHUB_BLOB_IMAGE = re.compile(r'(<img\s[^>]*src="https://github\.com/[^/"]+/[^/"]+)/blob/')
DTS_MARKER = "<!-- JsMxslc.d.ts -->"


def adapt_github_markdown(app, docname, source):
    """Adapts Markdown written for GitHub to Sphinx, leaving the files themselves GitHub-first."""
    text = source[0]

    # GitHub serves images from /blob/ URLs, other sites need the raw file.
    text = GITHUB_BLOB_IMAGE.sub(r"\1/raw/", text)

    # A centred <h1> title is not a heading to Sphinx.
    text = HTML_TITLE.sub(r"# \1", text, count=1)

    # GitHub docs often use several top-level headings, which Sphinx would list as
    # separate pages in the navigation. Keep the first as the page title and
    # move every heading after it down a level.
    lines = text.split("\n")
    headings = []
    in_code = False
    for i, line in enumerate(lines):
        if FENCE.match(line):
            in_code = not in_code
        elif not in_code and HEADING.match(line):
            headings.append(i)
    if sum(lines[i].startswith("# ") for i in headings) > 1:
        for i in headings[1:]:
            if not lines[i].startswith("######"):
                lines[i] = "#" + lines[i]

    source[0] = "\n".join(lines)


def insert_js_declarations(app, docname, source):
    if DTS_MARKER not in source[0]:
        return
    if JS_DTS.exists():
        path = os.path.relpath(JS_DTS, (Path(app.srcdir) / docname).parent)
        block = f"```{{literalinclude}} {path}\n:language: typescript\n```"
    else:
        logger.warning("%s not found, build the JavaScript module to generate the JavaScript API reference.", JS_DTS)
        block = "```{note}\nThe JavaScript module was not built, so its declarations are missing.\n```"
    source[0] = source[0].replace(DTS_MARKER, block)


def link_page_sources(app, pagename, templatename, context, doctree):
    """Points a page's view and edit buttons at its file in the repository."""
    if pagename.startswith("api/python/"):
        # Generated by sphinx-autoapi, there is no file.
        context["page_source_suffix"] = None
    elif not pagename.startswith(("docs/", "examples/")):
        # The site's own pages. The docs and examples are staged at their repository paths.
        context["theme_source_directory"] = "docs/site"


def copy_compiler(app, exception):
    if exception is not None or app.builder.format != "html":
        return
    if not (COMPILER / "lib" / "JsMxslc.js").exists():
        logger.warning("The browser compiler has no JsMxslc module in %s/lib, build the JavaScript module first.", COMPILER)
    shutil.copytree(
        COMPILER,
        Path(app.outdir) / "compiler",
        ignore=shutil.ignore_patterns("serve.js", "README.md"),
        dirs_exist_ok=True,
    )


def setup(app):
    app.connect("source-read", adapt_github_markdown)
    app.connect("source-read", insert_js_declarations)
    app.connect("html-page-context", link_page_sources)
    app.connect("build-finished", copy_compiler)
