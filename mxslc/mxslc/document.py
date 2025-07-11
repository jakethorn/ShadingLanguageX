from .mx_wrapper import Document


"""
The document being written to during compilation.
"""


_document = Document()


def get_document():
    return _document


def new_document():
    global _document
    _document = Document()
