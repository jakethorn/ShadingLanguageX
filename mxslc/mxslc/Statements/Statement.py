from __future__ import annotations
from abc import ABC, abstractmethod

from ..Attribute import Attribute
from ..DataType import DataType
from ..mx_wrapper import Node


class Statement(ABC):
    def __init__(self):
        self._attribs: list[Attribute] = []

    def add_attributes(self, attribs: list[Attribute]) -> None:
        self._attribs = attribs

    @abstractmethod
    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        ...

    @abstractmethod
    def execute(self) -> None:
        ...

    def _add_attributes_to_node(self, node: Node) -> None:
        for attrib in self._attribs:
            if attrib.child is None:
                node.set_attribute(attrib.name, attrib.value)
            else:
                child = node.get_child(attrib.child)
                child.set_attribute(attrib.name, attrib.value)
