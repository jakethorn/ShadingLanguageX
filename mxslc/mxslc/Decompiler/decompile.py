from pathlib import Path

from ..Argument import Argument
from ..DataType import BOOLEAN, INTEGER, FLOAT, MULTI_ELEM_TYPES, STRING, FILENAME
from ..Expressions import IdentifierExpression, LiteralExpression, Expression, ArithmeticExpression, \
    ComparisonExpression, IfExpression, LogicExpression, UnaryExpression, ConstructorCall, IndexingExpression, \
    SwitchExpression, FunctionCall, NodeConstructor
from ..Expressions.LiteralExpression import NullExpression
from ..Statements import VariableDeclaration, Statement
from ..Token import IdentifierToken, Token, LiteralToken
from ..file_utils import handle_input_path, handle_output_path
from ..mx_wrapper import Document, Node, Input


def decompile_file(mtlx_path: str | Path, mxsl_path: str | Path = None) -> None:
    mtlx_filepaths = handle_input_path(mtlx_path, extension=".mtlx")
    for mtlx_filepath in mtlx_filepaths:
        mxsl_filepath = handle_output_path(mxsl_path, mtlx_filepath, extension=".mxsl")
        decompiler = Decompiler(mtlx_filepath)
        mxsl = decompiler.decompile()
        with open(mxsl_filepath, "w") as f:
            f.write(mxsl)

        print(f"{mtlx_filepath.name} decompiled successfully.")


class Decompiler:
    def __init__(self, mtlx_filepath: Path):
        self.__doc = Document(mtlx_filepath)
        self.__nodes: list[Node] = self.__doc.get_nodes()
        self.__decompiled_nodes: list[Node] = []
        self.__mxsl = ""

    def decompile(self) -> str:
        self.__decompile(self.__nodes)
        return self.__mxsl

    def __decompile(self, nodes: list[Node]) -> None:
        for node in nodes:
            if node in self.__decompiled_nodes:
                continue
            self.__decompiled_nodes.append(node)
            input_nodes = [i.connected_node for i in node.inputs if i.connected_node]
            self.__decompile(input_nodes)
            self.__mxsl += f"{_deexecute(node)}\n"


def _deexecute(node: Node) -> Statement:
    identifier = IdentifierToken(node.name)
    expr = _node_to_expression(node)
    return VariableDeclaration(node.data_type, identifier, expr)


def _node_to_expression(node: Node) -> Expression:
    category = node.category
    data_type = node.data_type
    args = _inputs_to_arguments(node.inputs)
    if category == "constant":
        return _get_expression(args, 0)
    if category in ["convert", "combine2", "combine3", "combine4"]:
        return ConstructorCall(data_type, args)
    if category == "extract":
        return IndexingExpression(_get_expression(args, "in"), _get_expression(args, "index"))
    if category == "switch":
        values = [a.expression for a in args if "in" in a.name]
        return SwitchExpression(_get_expression(args, "which"), values)
    if category in _arithmetic_ops:
        return ArithmeticExpression(_get_expression(args, 0), Token(_arithmetic_ops[category]), _get_expression(args, 1))
    if category in _comparison_ops:
        expr = ComparisonExpression(_get_expression(args, "value1"), Token(_comparison_ops[category]), _get_expression(args, "value2"))
        if data_type == BOOLEAN and len(args) <= 2:
            return expr
        return IfExpression(expr, _get_expression(args, "in1"), _get_expression(args, "in2"))
    if category in _logic_ops:
        return LogicExpression(_get_expression(args, 0), Token(_logic_ops[category]), _get_expression(args, 1))
    if category in _unary_ops:
        return UnaryExpression(Token(_unary_ops[category]), _get_expression(args, "in"))
    if category in _stdlib_functions:
        return FunctionCall(IdentifierToken(category), None, args)
    category_token = LiteralToken(category)
    return NodeConstructor(category_token, data_type, args)


def _inputs_to_arguments(inputs: list[Input]) -> list[Argument]:
    args: list[Argument] = []
    for i, input_ in enumerate(inputs):
        arg_expression = _input_to_expression(input_)
        arg_identifier = IdentifierToken(input_.name)
        arg = Argument(arg_expression, i, arg_identifier)
        args.append(arg)
    return args


def _input_to_expression(input_: Input) -> Expression:
    node = input_.connected_node
    if node:
        node_identifier = IdentifierToken(node.name)
        return IdentifierExpression(node_identifier)
    if input_.data_type in [BOOLEAN, INTEGER, FLOAT, STRING, FILENAME]:
        token = LiteralToken(input_.literal)
        return LiteralExpression(token)
    if input_.data_type in MULTI_ELEM_TYPES:
        return ConstructorCall(input_.data_type, _value_to_arguments(input_.literal_string))
    raise AssertionError(f"Unknown input type: '{input_.data_type}'.")


def _value_to_arguments(vec_str: str) -> list[Argument]:
    channels = [float(c) for c in vec_str.split(",")]
    exprs = [LiteralExpression(LiteralToken(c)) for c in channels]
    args = [Argument(e, i) for i, e in enumerate(exprs)]
    return args


def _get_expression(args: list[Argument], index: int | str) -> Expression:
    if isinstance(index, int):
        if index < len(args):
            return args[index].expression
        return NullExpression()
    if isinstance(index, str):
        for arg in args:
            if arg.name == index:
                return arg.expression
        return NullExpression()
    raise AssertionError


def _get_stdlib_functions() -> set[str]:
    document = Document()
    document.load_standard_library()
    return {nd.node_string for nd in document.node_defs}


_stdlib_functions = _get_stdlib_functions()

_arithmetic_ops = {
    "add": "+",
    "subtract": "-",
    "multiply": "*",
    "divide": "/",
    "modulo": "%",
    "power": "^",
}

_comparison_ops = {
    "ifequal": "==",
    "ifgreater": ">",
    "ifgreatereq": ">=",
}

_logic_ops = {
    "and": "&",
    "or": "|",
}

_unary_ops = {
    "not": "!",
}
