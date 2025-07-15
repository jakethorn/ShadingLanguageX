from . import Expression
from .FunctionCall import FunctionCall
from .ConstructorCall import ConstructorCall
from .. import node_utils
from ..Argument import Argument
from ..CompileError import CompileError
from ..DataType import BOOLEAN, DataType
from ..Keyword import Keyword
from ..mx_wrapper import Node
from ..Token import Token, IdentifierToken


class BinaryExpression(FunctionCall):
    def __init__(self, left: Expression, op: Token, right: Expression):
        category = {
            "+": "add",
            "-": "subtract",
            "*": "multiply",
            "/": "divide",
            "%": "modulo",
            "^": "power",
            "!=": "ifequal",
            "==": "ifequal",
            ">": "ifgreater",
            "<": "ifgreatereq",
            ">=": "ifgreatereq",
            "<=": "ifgreater",
            "&": "and",
            Keyword.AND: "and",
            "|": "or",
            Keyword.OR: "or"
        }[op.type]
        func_identifier = IdentifierToken(category, op.file, op.line)
        super().__init__(func_identifier, None, [Argument(left, 0), Argument(right, 1)])
        self.__left = left
        self.__op = op
        self.__right = right

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        # TODO this allows the following: float * vec3 (for example)
        try:
            super()._init_subexpr(valid_types)
        except CompileError as e:
            if self.__op.type in ["+", "-", "*", "/", "%", "^"] and self.__right.is_initialized:
                converter = ConstructorCall(self.__right.data_type, [Argument(self.__left, 0)])
                argument = Argument(converter, 0)
                self._set_argument(argument)
                super()._init_subexpr(valid_types)
            else:
                raise e

    def _init(self, valid_types: set[DataType]) -> None:
        # TODO this allows the following: float * vec3 (for example)
        try:
            super()._init(valid_types)
        except CompileError as e:
            if self.__op.type in ["+", "-", "*", "/", "%", "^"]:
                converter = ConstructorCall(self.__right.data_type, [Argument(self.__left, 0)])
                argument = Argument(converter, 0)
                self._set_argument(argument)
                super()._init_subexpr(valid_types)
                super()._init(valid_types)
            else:
                raise e

    def _evaluate(self) -> Node:
        node = super()._evaluate()
        if self.__op in ["<", "<=", "!="]:
            not_node = node_utils.create("not", BOOLEAN)
            not_node.set_input("in", node)
            return not_node
        else:
            return node

    def __str__(self) -> str:
        return f"{self.__left} {self.__op} {self.__right}"