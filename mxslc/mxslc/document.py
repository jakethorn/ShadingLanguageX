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


def use_temporary_document() -> Document:
    global _document
    prev_document = _document
    _document = Document()
    return prev_document


def end_temporary_document(document: Document) -> Document:
    global _document
    temp_document = _document
    _document = document
    return temp_document
