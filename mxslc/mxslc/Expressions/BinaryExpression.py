from . import Expression
from .FunctionCall import FunctionCall
from .. import node_utils
from ..Argument import Argument
from ..DataType import BOOLEAN
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