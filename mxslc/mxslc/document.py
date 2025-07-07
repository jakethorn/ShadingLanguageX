from .mx_classes import Document

_document = Document()


def get_document():
    return _document


def new_document():
    global _document
    _document = Document()
