"""Builds the ShadingLanguageX site into docs/site/_build/html.

    pip install -r docs/site/requirements.txt    # and Doxygen, e.g. apt install doxygen
    python docs/site/build.py

Sphinx reads one source folder, but the site's pages are in three: the site's own pages
here, the Markdown docs in docs/ and the Hello SLX tutorial in examples/. They are copied
into _build/src at their repository paths, so links and images between them keep working,
and Sphinx builds that copy with the configuration in conf.py.

Arguments are passed to sphinx-build, e.g. -W to fail on warnings.
"""

import shutil
import sys
from pathlib import Path

from sphinx.cmd.build import build_main

SITE = Path(__file__).resolve().parent
ROOT = SITE.parents[1]
BUILD = SITE / "_build"
SOURCE = BUILD / "src"


def stage_sources():
    shutil.rmtree(SOURCE, ignore_errors=True)
    shutil.copytree(SITE, SOURCE, ignore=shutil.ignore_patterns("_build", "_templates", "*.py", "*.txt", "Doxyfile"))
    shutil.copytree(ROOT / "docs", SOURCE / "docs", ignore=shutil.ignore_patterns("site", "_build"))
    (SOURCE / "examples").mkdir()
    shutil.copy2(ROOT / "examples" / "HelloSLX.md", SOURCE / "examples")
    shutil.copytree(ROOT / "examples" / "screenshots", SOURCE / "examples" / "screenshots")


if __name__ == "__main__":
    stage_sources()
    sys.exit(build_main(["-c", str(SITE), *sys.argv[1:], str(SOURCE), str(BUILD / "html")]))
