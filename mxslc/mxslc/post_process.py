from pathlib import Path

from .DataType import FILENAME
from .document import get_document
from .mx_classes import GraphElement


def post_process() -> None:
    document = get_document()

    for graph in [document, *document.node_graphs]:
        #_add_material_node()

        _remove_dot_nodes(graph)
        #_combine_to_constant()
        _remove_constant_nodes(graph)
        #_remove_redundant_convert_nodes()
        #_remove_unused_inputs()


def _add_material_node() -> None:
    surfaceshader_nodes = mx_utils.get_nodes("standard_surface")
    displacementshader_nodes = mx_utils.get_nodes("displacement")
    if len(surfaceshader_nodes) > 0 or len(displacementshader_nodes) > 0:
        material_node = mx_utils.surfacematerial("mxsl_material")
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


def _remove_redundant_convert_nodes() -> None:
    cvt_nodes = mx_utils.get_nodes("convert")
    for cvt_node in cvt_nodes:
        if cvt_node.data_type == cvt_node.get_input("in").data_type:
            input_node = cvt_node.get_input("in").value
            for input_name, node in cvt_node.outputs:
                node.set_input(input_name, input_node)
            mx_utils.remove_node(cvt_node)
