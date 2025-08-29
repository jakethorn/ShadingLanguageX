from .Expression import Expression
from .ExpressionFacade import ExpressionFacade
from .NodeExpression import NodeExpression
from ..Keyword import Keyword
from ..Token import Token


class BinaryExpression(ExpressionFacade):
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
        }[op]
        expr = NodeExpression(op, category, [left, right])
        if op in ["<", "<=", "!="]:
            expr = NodeExpression(op, "not", [expr])
        super().__init__(expr, f"{left} {op} {right}")
