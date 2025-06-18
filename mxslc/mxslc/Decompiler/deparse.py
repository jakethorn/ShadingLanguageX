import re
from pathlib import Path

import MaterialX as mx

from ..Argument import Argument
from ..DataType import DataType, BOOLEAN
from ..Expressions import IdentifierExpression, LiteralExpression, Expression, ArithmeticExpression, \
    ComparisonExpression, IfExpression, LogicExpression, UnaryExpression, ConstructorCall, IndexingExpression, \
    SwitchExpression, FunctionCall, NodeConstructor
from ..Expressions.LiteralExpression import NullExpression
from ..Keyword import Keyword
from ..Statements import Statement, VariableDeclaration
from ..Token import IdentifierToken, Token
from ..token_types import STRING_LITERAL


def deparse(nodes: list[mx.Node]) -> list[Statement]:
    return [_de_execute(n) for n in nodes]


def _de_execute(node: mx.Node) -> VariableDeclaration:
    category = node.getCategory()
    identifier = IdentifierToken(node.getName())
    data_type = DataType(node.getType())
    args = _inputs_to_arguments(node)
    expr = _build_expression(category, data_type, args)
    return VariableDeclaration(data_type, identifier, expr)


def _build_expression(category: str, data_type: DataType, args: list[Argument]) -> Expression:
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

    if category in _get_stdlib_functions():
        return FunctionCall(IdentifierToken(category), None, args)

    category_token = Token(STRING_LITERAL, category)
    return NodeConstructor(category_token, data_type, args)


def _inputs_to_arguments(node: mx.Node) -> list[Argument]:
    args: list[Argument] = []
    inputs: list[mx.Input] = node.getInputs()
    for i, input_ in enumerate(inputs):
        node: mx.Node = input_.getConnectedNode()
        if node:
            node_identifier = IdentifierToken(node.getName())
            expr = IdentifierExpression(node_identifier)
        else:
            literal = IdentifierToken(input_.getValueString())
            expr = LiteralExpression(literal)
        arg_identifier = IdentifierToken(input_.getName())
        arg = Argument(expr, i, arg_identifier)
        args.append(arg)
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
    global _stdlib_functions
    if len(_stdlib_functions) == 0:
        stdlib_defs_path = Path(__file__).parent.parent / "stdlib" / "stdlib_defs.mxsl"
        with open(stdlib_defs_path) as f:
            stdlib_defs = f.read()
        _stdlib_functions = {f.replace('"', '') for f in re.findall('"[a-z_]+"', stdlib_defs)}
    return _stdlib_functions


_arithmetic_ops = {
    "add": "+",
    "subtract": "-",
    "multiply": "*",
    "divide": "/",
    "mod": "%",
    "pow": "^",
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

_stdlib_functions: set[str] = set()
