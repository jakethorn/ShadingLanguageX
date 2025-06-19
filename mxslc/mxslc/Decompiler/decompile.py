import re
from pathlib import Path

import MaterialX as mx

from ..Argument import Argument
from ..DataType import DataType, BOOLEAN, INTEGER, FLOAT, MULTI_ELEM_TYPES, STRING, FILENAME
from ..Expressions import IdentifierExpression, LiteralExpression, Expression, ArithmeticExpression, \
    ComparisonExpression, IfExpression, LogicExpression, UnaryExpression, ConstructorCall, IndexingExpression, \
    SwitchExpression, FunctionCall, NodeConstructor
from ..Expressions.LiteralExpression import NullExpression
from ..Keyword import Keyword
from ..Statements import VariableDeclaration, Statement
from ..Token import IdentifierToken, Token
from ..file_utils import handle_input_path, handle_output_path
from ..token_types import STRING_LITERAL, INT_LITERAL, FLOAT_LITERAL, FILENAME_LITERAL


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
        self.__doc: mx.Document = mx.createDocument()
        mx.readFromXmlFile(self.__doc, str(mtlx_filepath))
        self.__nodes: list[mx.Node] = self.__doc.getNodes()
        self.__decompiled_nodes: list[mx.Node] = []
        self.__mxsl = ""

    def decompile(self) -> str:
        self.__decompile(self.__nodes)
        return self.__mxsl

    def __decompile(self, nodes: list[mx.Node]) -> None:
        for node in nodes:
            if node in self.__decompiled_nodes:
                continue
            self.__decompiled_nodes.append(node)
            inputs: list[mx.Input] = node.getInputs()
            input_nodes: list[mx.Node] = [i.getConnectedNode() for i in inputs if i.getConnectedNode()]
            self.__decompile(input_nodes)
            line = f"{_deexecute(node)}\n"
            # TODO remove this check when SLX supports material types
            if line.startswith("material"):
                line = f"//{line}"
            self.__mxsl += line


def _deexecute(node: mx.Node) -> Statement:
    data_type = DataType(node.getType())
    identifier = IdentifierToken(node.getName())
    expr = _node_to_expression(node)
    return VariableDeclaration(data_type, identifier, expr)


def _node_to_expression(node: mx.Node) -> Expression:
    category = node.getCategory()
    data_type = DataType(node.getType())
    args = _inputs_to_arguments(node.getInputs())

    if category == "constant":
        return _get_expression(args, 0)
    if category in ["convert", "combine2", "combine3", "combine4"]:
        return ConstructorCall(data_type.as_token, args)
    if category == "extract":
        return IndexingExpression(_get_expression(args, "in"), _get_expression(args, "index"))
    if category == "switch":
        values = [a.expression for a in args if "in" in a.name]
        return SwitchExpression(Token(Keyword.SWITCH), _get_expression(args, "which"), values)
    if category in _arithmetic_ops:
        return ArithmeticExpression(_get_expression(args, 0), Token(_arithmetic_ops[category]), _get_expression(args, 1))
    if category in _comparison_ops:
        expr = ComparisonExpression(_get_expression(args, "value1"), Token(_comparison_ops[category]), _get_expression(args, "value2"))
        if data_type == BOOLEAN and len(args) <= 2:
            return expr
        return IfExpression(Token(Keyword.IF), expr, _get_expression(args, "in1"), _get_expression(args, "in2"))
    if category in _logic_ops:
        return LogicExpression(_get_expression(args, 0), Token(_logic_ops[category]), _get_expression(args, 1))
    if category in _unary_ops:
        return UnaryExpression(Token(_unary_ops[category]), _get_expression(args, "in"))
    if category in _stdlib_functions:
        return FunctionCall(IdentifierToken(category), None, args)
    category_token = Token(STRING_LITERAL, category)
    return NodeConstructor(category_token, data_type, args)


def _inputs_to_arguments(inputs: list[mx.Input]) -> list[Argument]:
    args: list[Argument] = []
    for i, input_ in enumerate(inputs):
        arg_expression = _input_to_expression(input_)
        arg_identifier = IdentifierToken(input_.getName())
        arg = Argument(arg_expression, i, arg_identifier)
        args.append(arg)
    return args


def _input_to_expression(input_: mx.Input) -> Expression:
    node: mx.Node = input_.getConnectedNode()
    if node:
        node_identifier = IdentifierToken(node.getName())
        return IdentifierExpression(node_identifier)
    data_type = DataType(input_.getType())
    if data_type == BOOLEAN:
        token = Token(Keyword.TRUE if input_.getValue() else Keyword.FALSE)
        return LiteralExpression(token)
    if data_type == INTEGER:
        token = Token(INT_LITERAL, input_.getValueString())
        return LiteralExpression(token)
    if data_type == FLOAT:
        value_str = input_.getValueString()
        token = Token(FLOAT_LITERAL, _format_float(value_str))
        return LiteralExpression(token)
    if data_type in MULTI_ELEM_TYPES:
        return ConstructorCall(data_type.as_token, _value_to_arguments(input_.getValueString()))
    if data_type == STRING:
        token = Token(STRING_LITERAL, '"' + input_.getValueString() + '"')
        return LiteralExpression(token)
    if data_type == FILENAME:
        token = Token(FILENAME_LITERAL, '"' + input_.getValueString() + '"')
        return LiteralExpression(token)
    raise AssertionError(f"Unknown input type: '{data_type}'.")


def _value_to_arguments(vec_str: str) -> list[Argument]:
    channels = [_format_float(c) for c in vec_str.split(",")]
    exprs = [LiteralExpression(Token(FLOAT_LITERAL, c)) for c in channels]
    args = [Argument(e, i) for i, e in enumerate(exprs)]
    return args


def _format_float(float_str: str) -> str:
    return str(float(float_str))


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
    stdlib_defs_path = Path(__file__).parent.parent / "stdlib" / "stdlib_defs.mxsl"
    with open(stdlib_defs_path) as f:
        stdlib_defs = f.read()
    return {f.replace('"', '') for f in re.findall('"[a-zA-Z0-9_]+"', stdlib_defs)}


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
