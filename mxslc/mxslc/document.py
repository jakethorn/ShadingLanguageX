from .mx_classes import Document, GraphElement

_document = Document()


def is_document(graph_element: GraphElement) -> bool:
    return graph_element.source is _document.source


def get_document():
    return _document


def clear_document():
    global _document
    _document = Document()
