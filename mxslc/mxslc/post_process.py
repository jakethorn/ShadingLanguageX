from pathlib import Path

from .DataType import FILENAME, MATERIAL
from .document import get_document
from .mx_classes import GraphElement


# TODO add postprocess to check remove unused inputs in nodegraphs
# TODO add postprocess to convert combine nodes to constant nodes
# TODO add postprocess to check ports only have one of value/node/output or interface_name
# TODO add postprocess to run validate
# TODO add Generated using ShadingLanguageX (github.com/jakethorn/ShadingLanguageX) at top of xml


def post_process() -> None:
    document = get_document()

    _add_material_node()
    for graph in [document, *document.node_graphs]:
        _remove_redundant_convert_nodes(graph)
        _remove_dot_nodes(graph)
        _remove_constant_nodes(graph)


def _add_material_node() -> None:
    document = get_document()
    surfaceshader_nodes = document.get_nodes("standard_surface")
    displacementshader_nodes = document.get_nodes("displacement")
    if len(surfaceshader_nodes) > 0 or len(displacementshader_nodes) > 0:
        material_node = document.add_node("surfacematerial", MATERIAL)
        if len(surfaceshader_nodes) > 0:
            material_node.set_input("surfaceshader", surfaceshader_nodes[-1])
        if len(displacementshader_nodes) > 0:
            material_node.set_input("displacementshader", displacementshader_nodes[-1])


def _remove_dot_nodes(graph: GraphElement) -> None:
    dot_nodes = graph.get_nodes("dot")
    for dot_node in dot_nodes:
        input_node = dot_node.get_input("in").value
        if input_node:
            for port in dot_node.downstream_ports:
                port.value = input_node
                output_string = dot_node.get_input("in").source.getOutputString()
                if output_string:
                    port.source.setOutputString(output_string)
        input_interface = dot_node.get_input("in").interface_name
        if input_interface:
            for port in dot_node.downstream_ports:
                port.interface_name = input_interface
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


def _remove_redundant_convert_nodes(graph: GraphElement) -> None:
    cvt_nodes = graph.get_nodes("convert")
    for cvt_node in cvt_nodes:
        if cvt_node.data_type == cvt_node.get_input("in").data_type:
            input_node = cvt_node.get_input("in").value
            for port in cvt_node.downstream_ports:
                port.value = input_node
            cvt_node.remove()
