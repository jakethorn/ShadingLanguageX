from abc import ABC, abstractmethod
from pathlib import Path
from typing import Any

from ..CompileError import CompileError
from ..Token import Token


class Expression(ABC):
    @abstractmethod
    def evaluate(self) -> Any:
        ...


class LiteralExpression(Expression):
    def __init__(self, literal: Token):
        self.__literal = literal

    def evaluate(self) -> Any:
        if isinstance(self.__literal.value, Path):
            return str(self.__literal.value)
        return self.__literal.value


class GroupingExpression(Expression):
    def __init__(self, expr: Expression):
        self.__expr = expr

    def evaluate(self) -> Any:
        return self.__expr.evaluate()


class UnaryExpression(Expression):
    def __init__(self, operator: Token, right: Expression):
        self.__operator = operator
        self.__right = right

    def evaluate(self) -> Any:
        r = self.__right.evaluate()
        return {
            "+": r,
            "-": -r,
            "!": not r,
            "not": not r,
        }[self.__operator.type]


class BinaryExpression(Expression):
    def __init__(self, left: Expression, operator: Token, right: Expression):
        self.__left = left
        self.__operator = operator
        self.__right = right

    def evaluate(self) -> Any:
        l = self.__left.evaluate()
        r = self.__right.evaluate()
        o = self.__operator.type
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
        if o == "and": return l and r
        if o == "|": return l or r
        if o == "or": return l or r
        raise CompileError(self.__operator.line, f"Invalid preprocessor expression: '{l} {o} {r}.")
