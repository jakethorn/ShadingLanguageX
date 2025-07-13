from . import state
from .DataType import DataType, MULTI_ELEM_TYPES, INTEGER, FLOAT, STRING, FILENAME, SHADER_TYPES
from .Keyword import Keyword
from .mx_wrapper import Node, Uniform, type_of, Output


def create(category: str, data_type: DataType | str) -> Node:
    """
    Add node to the current states graph element.
    """
    return state.get_graph().add_node(category, data_type)


def constant(value: Uniform) -> Node:
    """
    Add constant node to the current states graph element.
    """
    node = create("constant", type_of(value))
    node.set_input("value", value)
    return node


def extract(in_: Node, index: Node | int | str) -> Node:
    """
    Add extract node to the current states graph element.
    """
    assert in_.data_type in MULTI_ELEM_TYPES
    if isinstance(index, Node):
        assert index.data_type == INTEGER
    if isinstance(index, str):
        index = {"x": 0, "y": 1, "z": 2, "w": 3, "r": 0, "g": 1, "b": 2, "a": 3}[index]
    node = create("extract", FLOAT)
    node.set_input("in", in_)
    node.set_input("index", index)
    return node


def extract_all(in_: Node) -> list[Node]:
    """
    Add extract nodes to the current states graph element for each element in the incoming node.
    """
    if in_.data_type == FLOAT:
        return [in_]
    elif in_.data_type in MULTI_ELEM_TYPES:
        extract_nodes = []
        for i in range(in_.data_size):
            extract_nodes.append(extract(in_, i))
        return extract_nodes
    else:
        raise AssertionError


def combine(ins: list[Node], output_type: DataType) -> Node:
    """
    Add combine node to the current states graph element.
    """
    assert 2 <= len(ins) <= 4
    node = create(f"combine{len(ins)}", output_type)
    for i, in_ in enumerate(ins):
        node.set_input(f"in{i + 1}", in_)
    return node


def convert(in_: Node, output_type: DataType) -> Node:
    """
    Add convert node to the current states graph element.
    """
    unconvertable_types = [STRING, FILENAME, *SHADER_TYPES]
    assert in_.data_type not in unconvertable_types
    assert output_type not in unconvertable_types

    node = create("convert", output_type)
    node.set_input("in", in_)
    return node


def null(data_type: DataType) -> Node:
    """
    Add null node to the current states graph element.
    """
    return create(Keyword.NULL, data_type)


def dot(in_: Node | Output) -> Node:
    """
    Add dot node to the current states graph element.
    """
    node = create("dot", type_of(in_))
    node.set_input("in", in_)
    return node
