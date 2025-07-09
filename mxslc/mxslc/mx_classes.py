from __future__ import annotations

from pathlib import Path

import MaterialX as mx

from .DataType import DataType, BOOLEAN, INTEGER, FLOAT, VECTOR2, VECTOR3, VECTOR4, COLOR3, COLOR4, STRING, FILENAME
from .Keyword import Keyword
from .mx_types import Constant


"""
Pythonic wrappers around the generated MaterialX Python API.
"""


#
#   Element
#


class Element:
    def __init__(self, source: mx.Element):
        self.__source = source

    @property
    def source(self) -> mx.Element:
        return self.__source

    def create_valid_child_name(self, name: str) -> str:
        return self.source.createValidChildName(name)

    @property
    def category(self) -> str:
        return self.source.getCategory()

    @property
    def name(self) -> str:
        return self.source.getName()

    @property
    def parent(self) -> Element:
        return Element(self.source.getParent())

    def __str__(self) -> str:
        return str(self.source)

    def __eq__(self, other: Element) -> bool:
        return self.source == other.source


#
#   Typed Element
#


class TypedElement(Element):
    def __init__(self, source: mx.InterfaceElement):
        super().__init__(source)

    @property
    def source(self) -> mx.TypedElement:
        return super().source

    @property
    def data_type(self) -> DataType:
        return DataType(self.source.getType())

    @property
    def data_size(self) -> int:
        return self.data_type.size


#
#   Interface Element
#


class InterfaceElement(TypedElement):
    def __init__(self, source: mx.InterfaceElement):
        super().__init__(source)

    @property
    def source(self) -> mx.InterfaceElement:
        return super().source

    def has_input(self, name: str) -> bool:
        return self.source.getInput(name) is not None

    def add_input(self, name: str, value: Node | Output | Constant = None, data_type: DataType = None) -> Input:
        assert not self.has_input(name)
        assert value is not None or data_type is not None
        if value is not None and data_type is not None:
            assert type_of(value) == data_type
        name = self.create_valid_child_name(name)
        data_type_str = str(data_type or type_of(value))
        input_ = Input(self.source.addInput(name, data_type_str))
        if value is not None:
            input_.value = value
        else:
            input_.value = data_type.default()
        return input_

    def get_input(self, name: str) -> Input:
        assert self.has_input(name)
        return Input(self.source.getInput(name))

    @property
    def inputs(self) -> list[Input]:
        return [Input(i) for i in self.source.getInputs()]

    @property
    def input_count(self) -> int:
        return len(self.source.getInputs())

    def set_input(self, name: str, value: Node | Output | Constant | None) -> Input:
        if self.has_input(name):
            input_ = self.get_input(name)
            input_.value = value
        else:
            assert value is not None
            input_ = self.add_input(name, value)
        return input_

    def remove_input(self, name: str) -> None:
        assert self.has_input(name)
        self.source.removeInput(name)

    def has_output(self, name: str) -> bool:
        return self.source.getOutput(name) is not None

    def add_output(self, name: str, value: Node | Output | Constant = None, data_type: DataType = None) -> Output:
        assert not self.has_output(name)
        assert value is not None or data_type is not None
        if value is not None and data_type is not None:
            assert type_of(value) == data_type
        name = self.create_valid_child_name(name)
        data_type_str = str(data_type or type_of(value))
        output = Output(self.source.addOutput(name, data_type_str))
        if value is not None:
            output.value = value
        else:
            output.value = data_type.default()
        return output

    def get_output(self, name="out") -> Output:
        assert self.has_output(name)
        return Output(self.source.getOutput(name))

    @property
    def output(self) -> Output:
        return self.get_output()

    @property
    def outputs(self) -> list[Output]:
        return [Output(o) for o in self.source.getOutputs()]

    @property
    def output_count(self) -> int:
        return len(self.source.getOutputs())

    def set_output(self, name: str, value: Node | Output | Constant | None) -> Output:
        if self.has_output(name):
            output = self.get_output(name)
            output.value = value
        else:
            assert value is not None
            output = self.add_output(name, value)
        return output


#
#   Graph Element
#


class GraphElement(InterfaceElement):
    def __init__(self, source: mx.GraphElement):
        super().__init__(source)

    @property
    def source(self) -> mx.GraphElement:
        return super().source

    def add_node(self, category: str, data_type: DataType) -> Node:
        return Node(self.source.addNode(category, "", str(data_type)))

    def remove_node(self, name: str) -> None:
        self.source.removeNode(name)

    def get_nodes(self, category="") -> list[Node]:
        return [Node(n) for n in self.source.getNodes(category)]


#
#   Port Element
#


class PortElement(TypedElement):
    def __init__(self, source: mx.PortElement):
        super().__init__(source)

    @property
    def source(self) -> mx.PortElement:
        return super().source

    @property
    def parent(self) -> InterfaceElement:
        return InterfaceElement(self.source.getParent())

    @property
    def value(self) -> Node | Output | Constant:
        return Output(self.source.getConnectedOutput()) or Node(self.source.getConnectedNode()) or self.source.getValue()

    @value.setter
    def value(self, value: Node | Output | Constant | None) -> None:
        if value is None:
            self.parent.remove_input(self.name)
        elif isinstance(value, Node):
            if value.is_null_node:
                self.remove()
                value.remove()
            else:
                self.source.setConnectedNode(value.source)
        elif isinstance(value, Output):
            self.source.setConnectedOutput(value.source)
        else:
            self.source.setConnectedNode(None)
            if isinstance(value, Path):
                self.source.setValue(str(value), Keyword.FILENAME)
            else:
                self.source.setValue(value)

    @property
    def interface_name(self) -> str | None:
        return self.source.getInterfaceName()

    @interface_name.setter
    def interface_name(self, name: str) -> None:
        self.source.removeAttribute("value")
        self.source.setConnectedNode(None)
        self.source.setInterfaceName(name)

    def remove(self) -> None:
        self.parent.remove_input(self.name)


#
#   Document
#


class Document(GraphElement):
    def __init__(self, source: mx.Document | str | Path = None):
        if source is None:
            source = mx.createDocument()
        elif isinstance(source, str):
            doc = mx.createDocument()
            mx.readFromXmlString(doc, source)
            source = doc
        elif isinstance(source, Path):
            doc = mx.createDocument()
            mx.readFromXmlFile(doc, str(source))
            source = doc

        super().__init__(source)

    @property
    def source(self) -> mx.Document:
        return super().source

    def add_node_def(self, name: str, data_type: DataType, node_name: str) -> NodeDef:
        assert name.startswith("ND_")
        name = self.create_valid_child_name(name)
        node_def = NodeDef(self.source.addNodeDef(name, str(data_type), node_name))
        node_def.output.default = data_type.default()
        return node_def

    def add_node_graph_from_def(self, node_def: NodeDef) -> NodeGraph:
        name = node_def.name.replace("ND_", "NG_")
        node_graph = NodeGraph(self.source.addNodeGraph(name))
        node_graph.node_def = node_def
        return node_graph

    @property
    def node_defs(self) -> list[NodeDef]:
        return [NodeDef(nd) for nd in self.source.getNodeDefs()]

    @property
    def node_graphs(self) -> list[NodeGraph]:
        return [NodeGraph(ng) for ng in self.source.getNodeGraphs()]

    @property
    def xml(self) -> str:
        return mx.writeToXmlString(self.source)

    def validate(self) -> tuple[bool, str]:
        return self.source.validate()


#
#   Node
#


class Node(InterfaceElement):
    def __new__(cls, source: mx.Node):
        if source is None:
            return None
        else:
            return super().__new__(cls)

    def __init__(self, source: mx.Node):
        super().__init__(source)

    @property
    def source(self) -> mx.Node:
        return super().source

    @property
    def parent(self) -> GraphElement:
        return GraphElement(self.source.getParent())

    @property
    def name(self) -> str:
        return self.source.getName()

    @name.setter
    def name(self, name: str) -> None:
        if name != self.name:
            name = self.parent.create_valid_child_name(name)
            self.source.setName(name)

    @property
    def is_null_node(self) -> bool:
        return self.category == Keyword.NULL

    @property
    def downstream_ports(self) -> list[PortElement]:
        return [PortElement(p) for p in self.source.getDownstreamPorts()]

    def add_interface_input(self, input_name: str, data_type: DataType, interface_name: str) -> Input:
        input_ = self.add_input(input_name, data_type=data_type)
        input_.interface_name = interface_name
        return input_

    def remove(self) -> None:
        self.parent.remove_node(self.name)


#
#   Input
#


class Input(PortElement):
    def __new__(cls, source: mx.Input):
        if source is None:
            return None
        else:
            return super().__new__(cls)

    def __init__(self, source: mx.Input):
        super().__init__(source)


#
#   Output
#


class Output(PortElement):
    def __new__(cls, source: mx.Output):
        if source is None:
            return None
        else:
            return super().__new__(cls)

    def __init__(self, source: mx.Output):
        super().__init__(source)

    @property
    def default(self) -> str:
        return self.source.getAttribute("default")

    @default.setter
    def default(self, value: Constant) -> None:
        self.source.setAttribute("default", str(value))


#
#   NodeDef
#


class NodeDef(InterfaceElement):
    def __new__(cls, source: mx.NodeDef):
        if source is None:
            return None
        else:
            return super().__new__(cls)

    def __init__(self, source: mx.NodeDef):
        super().__init__(source)

    @property
    def source(self) -> mx.NodeDef:
        return super().source

    @property
    def parent(self) -> Document:
        return Document(self.source.getParent())

    @property
    def node_string(self) -> str:
        return self.source.getNodeString()


#
#   NodeGraph
#


class NodeGraph(GraphElement):
    def __new__(cls, source: mx.NodeDef):
        if source is None:
            return None
        else:
            return super().__new__(cls)

    def __init__(self, source: mx.NodeGraph):
        super().__init__(source)

    @property
    def source(self) -> mx.NodeGraph:
        return super().source

    @property
    def parent(self) -> Document:
        return Document(self.source.getParent())

    @property
    def node_def(self) -> NodeDef:
        return NodeDef(self.source.getNodeDef())

    @node_def.setter
    def node_def(self, node_def: NodeDef) -> None:
        self.source.setNodeDef(node_def.source)


#
#   util functions
#


def type_of(value: TypedElement | Constant) -> DataType:
    if isinstance(value, TypedElement):
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
