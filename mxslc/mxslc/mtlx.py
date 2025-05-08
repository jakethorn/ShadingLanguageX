from __future__ import annotations

from pathlib import Path
from typing import Any

import MaterialX as mx

from .Keyword import DataType, FILENAME, VECTOR_TYPES, COLOR_TYPES, FLOAT, STRING, SHADER_TYPES, BOOLEAN, INTEGER, \
    VECTOR2, VECTOR3, VECTOR4, COLOR3, COLOR4, MATERIAL

#
# Types
#


type Constant = bool | int | float | mx.Vector2 | mx.Vector3 | mx.Vector4 | mx.Color3 | mx.Color4 | str | Path


#
# Document
#


_document: mx.Document = mx.createDocument()


def get_document() -> mx.Document:
    return _document


def get_xml() -> str:
    return mx.writeToXmlString(_document)


def clear() -> None:
    global _document
    _document = mx.createDocument()


#
# Node
#


class Node:
    def __init__(self, source: mx.Node):
        self.__source = source

    @property
    def category(self) -> str:
        return self.__source.getCategory()

    @category.setter
    def category(self, category: str) -> None:
        self.__source.setCategory(category)

    @property
    def name(self) -> str:
        return self.__source.getName()

    @name.setter
    def name(self, name: str) -> None:
        name = _document.createValidChildName(name)
        self.__source.setName(name)

    @property
    def data_type(self) -> DataType:
        return DataType(self.__source.getType())

    @data_type.setter
    def data_type(self, data_type: DataType) -> None:
        self.__source.setType(data_type)

    @property
    def data_size(self) -> int:
        return self.data_type.size

    def output_count(self) -> int:
        return len(self.__source.getDownstreamPorts())

    def get_input(self, name: str) -> Any:
        input_node = self.__source.getConnectedNode(name)
        if input_node:
            return Node(input_node)
        else:
            return self.__source.getInputValue(name)

    # TODO maybe pass in the token instead of the value, so we dont have to do this filename dance
    def set_input(self, name: str, value: Any) -> None:
        self.__source.setConnectedNode(name, None)
        if isinstance(value, Node):
            self.__source.setConnectedNode(name, value.__source)
        else:
            if isinstance(value, Path):
                self.__source.setInputValue(name, str(value), FILENAME)
            else:
                self.__source.setInputValue(name, value)

    def has_input(self, name: str) -> bool:
        return self.__source.getConnectedNode(name) is not None or self.__source.getInputValue(name) is not None

    def get_input_data_type(self, name: str) -> DataType:
        return DataType(self.__source.getInput(name).getType())

    def set_input_data_type(self, name: str, data_type: DataType) -> str:
        return self.__source.getInput(name).setType(data_type)

    def get_outputs(self) -> list[tuple[str, Node]]:
        downstream_ports: list[mx.Input] = self.__source.getDownstreamPorts()
        return [(p.getName(), Node(p.getParent())) for p in downstream_ports]


def get_source(node: Node) -> mx.Node:
    return _document.getNode(node.name)


#
# Network Functions
#


def create_node(category: str, data_type: DataType, name="") -> Node:
    return Node(_document.addNode(category, name, data_type))


def create_material_node(name: str) -> Node:
    return create_node("surfacematerial", MATERIAL, name)


def remove_node(node: Node) -> None:
    _document.removeNode(node.name)


def get_nodes(category="") -> list[Node]:
    return [Node(n) for n in _document.getNodes(category)]


#
# Node Functions
#


def constant(value: Constant) -> Node:
    node = create_node("constant", type_of(value))
    node.set_input("value", value)
    return node


def extract(in_: Node, index: Node | int | str) -> Node:
    assert in_.data_type in [*VECTOR_TYPES, *COLOR_TYPES]
    if isinstance(index, Node):
        assert index.data_type is INTEGER
    if isinstance(index, str):
        index = {"x": 0, "y": 1, "z": 2, "w": 3, "r": 0, "g": 1, "b": 2, "a": 3}[index]
    node = create_node("extract", FLOAT)
    node.set_input("in", in_)
    node.set_input("index", index)
    return node


def extract_all(in_: Node) -> list[Node]:
    if in_.data_type == FLOAT:
        return [in_]
    elif in_.data_type in [*VECTOR_TYPES, *COLOR_TYPES]:
        extract_nodes = []
        for i in range(in_.data_size):
            extract_nodes.append(extract(in_, i))
        return extract_nodes
    else:
        raise AssertionError


def combine(ins: list[Node], output_type: DataType) -> Node:
    assert 2 <= len(ins) <= 4
    node = create_node(f"combine{len(ins)}", output_type)
    for i, in_ in enumerate(ins):
        node.set_input(f"in{i+1}", in_)
    return node


def convert(in_: Node, output_type: DataType) -> Node:
    unconvertable_types = [STRING, FILENAME, *SHADER_TYPES]
    assert in_.data_type not in unconvertable_types
    assert output_type not in unconvertable_types

    node = create_node("convert", output_type)
    node.set_input("in", in_)
    return node


#
# Util functions
#


def type_of(value: Node | Constant) -> DataType:
    if isinstance(value, Node):
        return value.data_type
    if isinstance(value, bool):
        return BOOLEAN
    if isinstance(value, int):
        return INTEGER
    if isinstance(value, float):
        return FLOAT
    if isinstance(value, mx.Vector2):
        return VECTOR2
    if isinstance(value, mx.Vector3):
        return VECTOR3
    if isinstance(value, mx.Vector4):
        return VECTOR4
    if isinstance(value, mx.Color3):
        return COLOR3
    if isinstance(value, mx.Color4):
        return COLOR4
    if isinstance(value, str):
        return STRING
    if isinstance(value, Path):
        return FILENAME
    raise AssertionError
