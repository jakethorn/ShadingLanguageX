from .Expression import Expression, BinaryExpression, UnaryExpression, LiteralExpression, GroupingExpression, Primitive, \
    TernaryRelationalExpression
from ..CompileError import CompileError
from ..Keyword import Keyword
from ..Token import Token
from ..TokenReader import TokenReader
from ..token_types import INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, FILENAME_LITERAL


def parse(tokens: list[Token]) -> Primitive:
    return Parser(tokens).parse()


class Parser(TokenReader):
    def __init__(self, tokens: list[Token]):
        super().__init__(tokens)

    def parse(self) -> Primitive:
        expr = self.__expression()
        return expr.evaluate()

    def __expression(self) -> Expression:
        return self.__logic()

    def __logic(self) -> Expression:
        expr = self.__equality()
        while op := self._consume("&", Keyword.AND, "|", Keyword.OR):
            right = self.__equality()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __equality(self) -> Expression:
        expr = self.__relational()
        while op := self._consume("!=", "=="):
            right = self.__relational()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __relational(self) -> Expression:
        left = self.__term()
        middle = None
        right = None

        relational_operators = [">", ">=", "<", "<="]
        if op1 := self._consume(relational_operators):
            middle = self.__term()
        if op2 := self._consume(relational_operators):
            right = self.__term()

        if middle is None:
            return left
        elif right is None:
            return BinaryExpression(left, op1, middle)
        else:
            return TernaryRelationalExpression(left, op1, middle, op2, right)

    def __term(self) -> Expression:
        expr = self.__factor()
        while op := self._consume("+", "-"):
            right = self.__factor()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __factor(self) -> Expression:
        expr = self.__exponent()
        while op := self._consume("*", "/", "%"):
            right = self.__exponent()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __exponent(self) -> Expression:
        expr = self.__unary()
        while op := self._consume("^"):
            right = self.__unary()
            expr = BinaryExpression(expr, op, right)
        return expr

    def __unary(self) -> Expression:
        if op := self._consume("!", Keyword.NOT, "+", "-"):
            return UnaryExpression(op, self.__primary())
        else:
            return self.__primary()

    def __primary(self) -> Expression:
        if literal := self._consume(Keyword.TRUE, Keyword.FALSE, INT_LITERAL, FLOAT_LITERAL, STRING_LITERAL, FILENAME_LITERAL):
            return LiteralExpression(literal)
        if self._consume("("):
            expr = self.__expression()
            self._match(")")
            return GroupingExpression(expr)
        token = self._peek()
        raise CompileError(f"Unexpected token: '{token}'.", token)
