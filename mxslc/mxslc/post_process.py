from pathlib import Path

from . import mx_utils
from .DataType import BOOLEAN, FLOAT
from .Keyword import Keyword
from .utils import is_path


def post_process() -> None:
    _add_material_node()
    _remove_dot_nodes()
    _remove_null_nodes()
    _remove_constant_nodes()
    _fix_logic_nodes()
    _remove_convert_nodes()


def _add_material_node() -> None:
    surfaceshader_nodes = mx_utils.get_nodes("standard_surface")
    displacementshader_nodes = mx_utils.get_nodes("displacement")
    if len(surfaceshader_nodes) > 0 or len(displacementshader_nodes) > 0:
        material_node = mx_utils.create_material_node("mxsl_material")
        if len(surfaceshader_nodes) > 0:
            material_node.set_input("surfaceshader", surfaceshader_nodes[-1])
        if len(displacementshader_nodes) > 0:
            material_node.set_input("displacementshader", displacementshader_nodes[-1])


# TODO this function isnt being called
# TODO loop until nothing is removed
def _remove_nodes_with_no_outputs() -> None:
    nodes = mx_utils.get_nodes()
    for node in nodes:
        if node.category == "surfacematerial":
            continue
        if node.output_count() == 0:
            mx_utils.remove_node(node)


def _remove_dot_nodes() -> None:
    dot_nodes = mx_utils.get_nodes("dot")
    for dot_node in dot_nodes:
        input_node = dot_node.get_input("in")
        for input_name, node in dot_node.get_outputs():
            node.set_input(input_name, input_node)
        mx_utils.remove_node(dot_node)


def _remove_null_nodes() -> None:
    null_nodes = mx_utils.get_nodes(Keyword.NULL)
    for null_node in null_nodes:
        mx_utils.remove_node(null_node)


def _remove_constant_nodes() -> None:
    constant_nodes = mx_utils.get_nodes("constant")
    for constant_node in constant_nodes:
        input_value = constant_node.get_input("value")
        if is_path(input_value):
            input_value = Path(input_value)
        for input_name, node in constant_node.get_outputs():
            node.set_input(input_name, input_value)
        mx_utils.remove_node(constant_node)


def _remove_logic_nodes() -> None:
    removable_nodes = set()
    ifequal_nodes = mx_utils.get_nodes("ifequal")
    ifequal_nodes = [n for n in ifequal_nodes if n.data_type != BOOLEAN]
    for ifequal_node in ifequal_nodes:
        comparison_node = ifequal_node.get_input("value1")
        ifequal_node.category = comparison_node.category
        ifequal_node.set_input("value1", comparison_node.get_input("value1"))
        ifequal_node.set_input("value2", comparison_node.get_input("value2"))
        removable_nodes.add(comparison_node)
    for node in removable_nodes:
        mx_utils.remove_node(node)


def _fix_logic_nodes() -> None:
    # and
    and_nodes = mx_utils.get_nodes("and")
    for and_node in and_nodes:
        and_node.category = "min"
        and_node.data_type = FLOAT
        _fix_logic_node_input(and_node, "in1")
        _fix_logic_node_input(and_node, "in2")

    # or
    or_nodes = mx_utils.get_nodes("or")
    for or_node in or_nodes:
        or_node.category = "max"
        or_node.data_type = FLOAT
        _fix_logic_node_input(or_node, "in1")
        _fix_logic_node_input(or_node, "in2")

    # not
    not_nodes = mx_utils.get_nodes("not")
    for not_node in not_nodes:
        not_node.category = "invert"
        not_node.data_type = FLOAT
        _fix_logic_node_input(not_node, "in")

    # ifgreater
    gt_nodes = mx_utils.get_nodes("ifgreater")
    for gt_node in gt_nodes:
        gt_node.data_type = FLOAT
        gt_node.set_input("in1", 1.0)
        gt_node.set_input("in2", 0.0)

    # ifgreatereq
    ge_nodes = mx_utils.get_nodes("ifgreatereq")
    for ge_node in ge_nodes:
        ge_node.data_type = FLOAT
        ge_node.set_input("in1", 1.0)
        ge_node.set_input("in2", 0.0)

    # ifequal
    eq_nodes = mx_utils.get_nodes("ifequal")
    for eq_node in eq_nodes:
        if eq_node.has_input("in1") and eq_node.has_input("in2"):
            _fix_logic_node_input(eq_node, "value1")
            eq_node.set_input("value2", 1.0)
        else:
            eq_node.data_type = FLOAT
            _fix_logic_node_input(eq_node, "value1")
            _fix_logic_node_input(eq_node, "value2")
            eq_node.set_input("in1", 1.0)
            eq_node.set_input("in2", 0.0)

    # convert
    cvt_nodes = mx_utils.get_nodes("convert")
    for cvt_node in cvt_nodes:
        _fix_logic_node_input(cvt_node, "in")


def _remove_convert_nodes() -> None:
    cvt_nodes = mx_utils.get_nodes("convert")
    for cvt_node in cvt_nodes:
        if cvt_node.data_type == cvt_node.get_input_data_type("in"):
            input_node = cvt_node.get_input("in")
            for input_name, node in cvt_node.get_outputs():
                node.set_input(input_name, input_node)
            mx_utils.remove_node(cvt_node)


def _fix_logic_node_input(node: mx_utils.Node, input_name: str) -> None:
    if node.get_input_data_type(input_name) == BOOLEAN:
        input_value = node.get_input(input_name)
        if isinstance(input_value, mx_utils.Node):
            node.set_input_data_type(input_name, FLOAT)
        else:
            node.set_input(input_name, float(input_value))
