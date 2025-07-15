from . import VariableAssignment
from ..Expressions import Expression, IdentifierExpression, SwizzleExpression, BinaryExpression
from ..Token import Token


class CompoundAssignment(VariableAssignment):
    def __init__(self, identifier: Token, swizzle: Token | None, operator: Token, right: Expression):
        left = IdentifierExpression(identifier)
        if swizzle:
            left = SwizzleExpression(left, swizzle)
        binary_op = Token(operator.lexeme[0])
        expr = BinaryExpression(left, binary_op, right)
        super().__init__(identifier, swizzle, expr)
