from __future__ import annotations

from . import state
from .DataType import MATERIAL, SHADER_TYPES, MULTI_ELEM_TYPES
from .mx_classes import *


def constant(value: Uniform) -> Node:
    node = state.add_unnamed_node("constant", type_of(value))
    node.set_input("value", value)
    return node


def extract(in_: Node, index: Node | int | str) -> Node:
    assert in_.data_type in MULTI_ELEM_TYPES
    if isinstance(index, Node):
        assert index.data_type == INTEGER
    if isinstance(index, str):
        index = {"x": 0, "y": 1, "z": 2, "w": 3, "r": 0, "g": 1, "b": 2, "a": 3}[index]
    node = state.add_unnamed_node("extract", FLOAT)
    node.set_input("in", in_)
    node.set_input("index", index)
    return node


def extract_all(in_: Node) -> list[Node]:
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
    assert 2 <= len(ins) <= 4
    node = state.add_unnamed_node(f"combine{len(ins)}", output_type)
    for i, in_ in enumerate(ins):
        node.set_input(f"in{i + 1}", in_)
    return node


def convert(in_: Node, output_type: DataType) -> Node:
    unconvertable_types = [STRING, FILENAME, *SHADER_TYPES]
    assert in_.data_type not in unconvertable_types
    assert output_type not in unconvertable_types

    node = state.add_unnamed_node("convert", output_type)
    node.set_input("in", in_)
    return node


def surfacematerial() -> Node:
    return state.add_unnamed_node("surfacematerial", MATERIAL)


def null_node(data_type: DataType) -> Node:
    return state.add_unnamed_node(Keyword.NULL, data_type)
