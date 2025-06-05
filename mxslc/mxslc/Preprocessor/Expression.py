from abc import ABC, abstractmethod
from pathlib import Path

from ..CompileError import CompileError
from ..Keyword import Keyword
from ..Token import Token

type Primitive = bool | int | float | str


class Expression(ABC):
    @abstractmethod
    def evaluate(self) -> Primitive:
        ...


class LiteralExpression(Expression):
    def __init__(self, literal: Token):
        self.__literal = literal

    def evaluate(self) -> Primitive:
        if isinstance(self.__literal.value, Path):
            return str(self.__literal.value)
        return self.__literal.value


class GroupingExpression(Expression):
    def __init__(self, expr: Expression):
        self.__expr = expr

    def evaluate(self) -> Primitive:
        return self.__expr.evaluate()


class UnaryExpression(Expression):
    def __init__(self, op: Token, right: Expression):
        self.__op = op
        self.__right = right

    def evaluate(self) -> Primitive:
        r = self.__right.evaluate()
        o = self.__op.type
        if o == "+": return r
        if o == "-": return -r
        if o == "!": return not r
        if o == Keyword.NOT: return not r
        raise CompileError(f"Invalid preprocessor expression: '{o}{r}'.", self.__op)


class BinaryExpression(Expression):
    def __init__(self, left: Expression, op: Token, right: Expression):
        self.__left = left
        self.__op = op
        self.__right = right

    def evaluate(self) -> Primitive:
        l = self.__left.evaluate()
        r = self.__right.evaluate()
        o = self.__op.type
        if o == "+": return l + r
        if o == "-": return l - r
        if o == "*": return l * r
        if o == "/": return l / r
        if o == "^": return l ** r
        if o == ">": return l > r
        if o == ">=": return l >= r
        if o == "<": return l < r
        if o == "<=": return l <= r
        if o == "==": return l == r
        if o == "!=": return l != r
        if o == "&": return l and r
        if o == Keyword.AND: return l and r
        if o == "|": return l or r
        if o == Keyword.OR: return l or r
        raise CompileError(f"Invalid preprocessor expression: '{l} {o} {r}'.", self.__op)


class TernaryRelationalExpression(Expression):
    def __init__(self, left: Expression, op1: Token, middle: Expression, op2: Token, right: Expression):
        comp1 = BinaryExpression(left, op1, middle)
        comp2 = BinaryExpression(middle, op2, right)
        self.__and = BinaryExpression(comp1, Token(Keyword.AND), comp2)

    def evaluate(self) -> Primitive:
        return self.__and.evaluate()
