from . import VariableAssignment
from ..Expressions import Expression, IdentifierExpression, SwizzleExpression, ArithmeticExpression, LogicExpression
from ..Token import Token
from ..utils import string


class CompoundAssignment(VariableAssignment):
    def __init__(self, identifier: Token, swizzle: Token | None, operator: Token, right: Expression):
        left = IdentifierExpression(identifier)
        if swizzle:
            left = SwizzleExpression(left, swizzle)

        binary_op = Token(operator.lexeme[0])
        if binary_op in ["+", "-", "*", "/", "%", "^"]:
            expr = ArithmeticExpression(left, binary_op, right)
        else:
            expr = LogicExpression(left, binary_op, right)

        super().__init__(identifier, swizzle, expr)
