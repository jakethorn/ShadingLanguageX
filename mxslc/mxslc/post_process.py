from pathlib import Path

from .DataType import FILENAME
from .document import get_document
from .mx_wrapper import GraphElement, Input


# TODO add postprocess to check remove unused inputs in nodegraphs
# TODO add postprocess to convert combine nodes to constant nodes
# TODO add Generated using ShadingLanguageX (github.com/jakethorn/ShadingLanguageX) at top of xml


def post_process() -> None:
    document = get_document()

    for graph in [document, *document.node_graphs]:
        _remove_redundant_convert_nodes(graph)
        _remove_dot_nodes(graph)
        _remove_constant_nodes(graph)


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
