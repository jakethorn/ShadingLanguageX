
from typing import Union
from pathlib import Path

from .DecompileError import DecompileError
from ..Argument import Argument
from ..DataType import BOOLEAN, INTEGER, FLOAT, MULTI_ELEM_TYPES, STRING, FILENAME, VOID
from ..Expressions import IdentifierExpression, LiteralExpression, Expression, IfExpression, UnaryExpression, \
    ConstructorCall, IndexingExpression, SwitchExpression, FunctionCall, NodeConstructor, BinaryExpression
from ..Expressions.LiteralExpression import NullExpression
from ..Expressions.VariableDeclarationExpression import VariableDeclarationExpression
from ..Statements import VariableDeclaration, ExpressionStatement, Statement
from ..Token import IdentifierToken, Token, LiteralToken
from ..file_utils import handle_input_path, handle_output_path
from ..mx_wrapper import Document, Node, Input, Output


def decompile_file(mtlx_path: Union[str, Path], mxsl_path: Union[str, Path, None] = None) -> None:
    mtlx_filepaths = handle_input_path(mtlx_path, extension=".mtlx")
    for mtlx_filepath in mtlx_filepaths:
        mxsl_filepath = handle_output_path(mxsl_path, mtlx_filepath, extension=".mxsl")
        decompiler = Decompiler(mtlx_filepath)
        mxsl = decompiler.decompile()
        with open(mxsl_filepath, "w") as f:
            f.write(mxsl)

        print(f"{mtlx_filepath.name} decompiled successfully.")

def decompile_string(mtlx_content: str) -> str:    
    decompiler = Decompiler(mtlx_content)
    return decompiler.decompile()

class Decompiler:
    def __init__(self, mtlx_filepath: Union[str, Path]):
        self.__doc = Document(mtlx_filepath)
        self.__doc.load_standard_library()
        self.__func_names = {nd.node_string for nd in self.__doc.node_defs}
        self.__nodes: list[Node] = self.__doc.get_nodes()
        self.__decompiled_nodes: list[Node] = []
        self.__mxsl = ""

    def decompile(self) -> str:
        self.__decompile_nodes(self.__nodes)
        return self.__mxsl

    def __decompile_nodes(self, nodes: list[Node]) -> None:
        for node in nodes:
            if node in self.__decompiled_nodes:
                continue
            self.__decompiled_nodes.append(node)
            input_nodes = [i.connected_node for i in node.inputs if i.connected_node]
            self.__decompile_nodes(input_nodes)
            self.__mxsl += f"{self.__deexecute(node)}\n"

    def __deexecute(self, node: Node) -> Statement:
        identifier = IdentifierToken(node.name)
        expr = self.__node_to_expression(node)
        if node.data_type == VOID:
            return ExpressionStatement(expr)
        else:
            return VariableDeclaration([], node.data_type, identifier, expr)

    def __node_to_expression(self, node: Node) -> Expression:
        category = node.category
        args = _inputs_to_arguments(node.inputs)
        if category == "constant":
            return _get_expression(args, 0)
        if category in ["convert", "combine2", "combine3", "combine4"]:
            return ConstructorCall(node.data_type, args)
        if category == "extract":
            return IndexingExpression(_get_expression(args, "in"), _get_expression(args, "index"))
        if category == "switch":
            values = [a.expression for a in args if "in" in a.name]
            return SwitchExpression(_get_expression(args, "which"), values)
        if category in _arithmetic_ops:
            return BinaryExpression(_get_expression(args, 0), Token(_arithmetic_ops[category]), _get_expression(args, 1))
        if category in _comparison_ops:
            expr = BinaryExpression(_get_expression(args, "value1"), Token(_comparison_ops[category]), _get_expression(args, "value2"))
            if node.data_type == BOOLEAN and len(args) <= 2:
                return expr
            return IfExpression(expr, _get_expression(args, "in1"), _get_expression(args, "in2"))
        if category in _logic_ops:
            return BinaryExpression(_get_expression(args, 0), Token(_logic_ops[category]), _get_expression(args, 1))
        if category in _unary_ops:
            return UnaryExpression(Token(_unary_ops[category]), _get_expression(args, "in"))
        if category in self.__func_names:
            outputs = node.get_node_def().outputs
            func_args = _outputs_to_arguments(node, outputs) + args
            return FunctionCall(IdentifierToken(category), None, func_args)
        category_token = LiteralToken(category)
        return NodeConstructor(category_token, node.data_type, args)


def _inputs_to_arguments(inputs: list[Input]) -> list[Argument]:
    args: list[Argument] = []
    for i, input_ in enumerate(inputs):
        expr = _input_to_expression(input_)
        identifier = IdentifierToken(input_.name)
        arg = Argument(expr, i, identifier)
        args.append(arg)
    return args


def _input_to_expression(input_: Input) -> Expression:
    node = input_.connected_node
    if node:
        name = node.name + (f"_{input_.output_string}" if input_.output_string else "")
        node_identifier = IdentifierToken(name)
        return IdentifierExpression(node_identifier)
    if input_.data_type in [BOOLEAN, INTEGER, FLOAT, STRING, FILENAME]:
        token = LiteralToken(input_.literal)
        return LiteralExpression(token)
    if input_.data_type in MULTI_ELEM_TYPES:
        return ConstructorCall(input_.data_type, _value_to_arguments(input_.literal_string))
    raise DecompileError(f"Invalid input", input_)


def _value_to_arguments(vec_str: str) -> list[Argument]:
    channels = [float(c) for c in vec_str.split(",")]
    exprs = [LiteralExpression(LiteralToken(c)) for c in channels]
    args = [Argument(e, i) for i, e in enumerate(exprs)]
    return args


def _outputs_to_arguments(node: Node, outputs: list[Output]) -> list[Argument]:
    if len(outputs) < 2:
        return []
    args: list[Argument] = []
    for i, output in enumerate(outputs):
        expr_identifier = IdentifierToken(f"{node.name}_{output.name}")
        expr = VariableDeclarationExpression(output.data_type, expr_identifier)
        arg_identifier = IdentifierToken(output.name)
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
