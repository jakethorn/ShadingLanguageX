from pathlib import Path

import MaterialX as mx

from .DataType import FILENAME, VECTOR3, VECTOR4, FLOAT
from .document import get_document
from .mx_wrapper import GraphElement, Node


# TODO add postprocess to remove unused inputs in nodegraphs
# TODO these postprocesses can slow down compilation for larger shaders and can probably be done as nodes are added --
# TODO -- to the network instead of going through the entire network at the end.


def post_process() -> None:
    document = get_document()

    for graph in [document, *document.node_graphs]:
        _remove_redundant_convert_nodes(graph)
        _remove_dot_nodes(graph)
        _remove_constant_nodes(graph)
        _remove_combine_extract_nodes(graph)


def _remove_redundant_convert_nodes(graph: GraphElement) -> None:
    cvt_nodes = graph.get_nodes("convert")
    for cvt_node in cvt_nodes:
        cvt_input = cvt_node.get_input("in")
        if cvt_node.data_type == cvt_input.data_type:
            for port in cvt_node.downstream_ports:
                port.value = cvt_input.value
            cvt_node.remove()


def _remove_dot_nodes(graph: GraphElement) -> None:
    dot_nodes = graph.get_nodes("dot")
    for dot_node in dot_nodes:
        dot_input = dot_node.get_input("in")
        if dot_input.value is not None:
            for port in dot_node.downstream_ports:
                port.value = dot_input.value
                port.output_string = dot_input.output_string
            dot_node.remove()
        elif dot_input.node_name is not None:
            for port in dot_node.downstream_ports:
                port.node_name = dot_input.node_name
            dot_node.remove()
        elif dot_input.interface_name is not None:
            for port in dot_node.downstream_ports:
                if not port.is_output:
                    port.interface_name = dot_input.interface_name
            if len(dot_node.downstream_ports) == 0:
                dot_node.remove()


def _remove_constant_nodes(graph: GraphElement) -> None:
    const_nodes = graph.get_nodes("constant")
    for const_node in const_nodes:
        input_value = const_node.get_input("value").value
        if const_node.data_type == FILENAME:
            input_value = Path(input_value)
        for port in const_node.downstream_ports:
            port.value = input_value
        const_node.remove()


def _remove_combine_extract_nodes(graph: GraphElement) -> None:
    for node in graph.get_nodes():
        if node.category == "combine2":
            _remove_combine2_node(node)
        if node.category == "combine3":
            _remove_combine3_node(node)
        if node.category == "combine4":
            _remove_combine4_node(node)
        if node.category == "extract":
            _remove_extract_node(node)


def _remove_combine2_node(node: Node) -> None:
    literals: list[float] = [i.literal for i in node.inputs if i.has_literal and i.data_type == FLOAT]
    if len(literals) < 2:
        return
    value = mx.Vector2(*literals)
    for port in node.downstream_ports:
        port.value = value
    node.remove()


def _remove_combine3_node(node: Node) -> None:
    literals: list[float] = [i.literal for i in node.inputs if i.has_literal and i.data_type == FLOAT]
    if len(literals) < 3:
        return
    if node.data_type == VECTOR3:
        value = mx.Vector3(*literals)
    else:
        value = mx.Color3(*literals)
    for port in node.downstream_ports:
        port.value = value
    node.remove()


def _remove_combine4_node(node: Node) -> None:
    literals: list[float] = [i.literal for i in node.inputs if i.has_literal and i.data_type == FLOAT]
    if len(literals) < 4:
        return
    if node.data_type == VECTOR4:
        value = mx.Vector4(*literals)
    else:
        value = mx.Color4(*literals)
    for port in node.downstream_ports:
        port.value = value
    node.remove()


def _remove_extract_node(node: Node) -> None:
    if node.has_input("in") and node.has_input("index"):
        in_ = node.get_input("in")
        index = node.get_input("index")
        if in_.has_literal and index.has_literal:
            value = in_.literal[index.literal]
            for port in node.downstream_ports:
                port.value = value
            node.remove()
