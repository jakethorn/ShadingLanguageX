"""Opens the ShadingLanguageX documentation in a web browser."""

import webbrowser

import hou

REPOSITORY_URL = "https://github.com/jakethorn/ShadingLanguageX"
DOCUMENTS = (
    ("SLX Github", REPOSITORY_URL),
    ("SLX Language Specification", REPOSITORY_URL + "/blob/main/docs/LanguageSpecification.md"),
    ("SLX Plugin Docs", REPOSITORY_URL + "/blob/main/tools/houdini/README.md"),
)


def open_documentation():
    """Asks which documentation to open, then opens it in a web browser."""
    labels = [label for label, _ in DOCUMENTS]
    cancel = len(labels)
    choice = hou.ui.displayMessage("Which documentation would you like to open?", buttons=labels + ["Cancel"],
                                   close_choice=cancel, title="SLX Docs")
    if choice != cancel:
        webbrowser.open(DOCUMENTS[choice][1])
