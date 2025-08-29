from . import Expression
from .ExpressionFacade import ExpressionFacade
from .LiteralExpression import NullExpression
from .NodeExpression import NodeExpression
from ..Keyword import Keyword
from ..Token import Token
from ..mx_wrapper import Uniform


class UnaryExpression(ExpressionFacade):
    def __init__(self, op: Token, right: Expression):
        if op in ["!", Keyword.NOT]:
            expr = NodeExpression(op, "not", [right])
        elif op == "-":
            expr = NodeExpression(op, "subtract", [NullExpression(), right])
        else:
            expr = right
        super().__init__(expr, f"{op} {right}")
        self.__op = op
        self.__right = right

    @property
    def _value(self) -> Uniform | None:
        if self.__op == "-":
            if self.__right.has_value:
                return -self.__right.value
            else:
                return None
        else:
            return super()._value
