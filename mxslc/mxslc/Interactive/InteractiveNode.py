from __future__ import annotations

from typing import Type

import MaterialX as mx

from .InteractiveExpression import InteractiveExpression
from .mx_interactive_types import Value
from ..Expressions import ArithmeticExpression, LogicExpression, ComparisonExpression, UnaryExpression, IndexingExpression
from ..Token import Token
from ..mx_wrapper import Node


class InteractiveNode:
    def __init__(self, node: mx.Node | Node):
        if isinstance(node, mx.Node):
            self.__node = node
        else:
            self.__node = node.source

    @property
    def node(self) -> mx.Node:
        return self.__node

    def __add__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ArithmeticExpression, self.__node, "+", other)

    def __sub__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ArithmeticExpression, self.__node, "-", other)

    def __mul__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ArithmeticExpression, self.__node, "*", other)

    def __truediv__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ArithmeticExpression, self.__node, "/", other)

    def __pow__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ArithmeticExpression, self.__node, "^", other)

    def __mod__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ArithmeticExpression, self.__node, "%", other)

    def __and__(self, other: Value) -> InteractiveNode:
        return _binary_expr(LogicExpression, self.__node, "&", other)

    def __or__(self, other: Value) -> InteractiveNode:
        return _binary_expr(LogicExpression, self.__node, "|", other)

    def __xor__(self, other: Value) -> InteractiveNode:
        return _binary_expr(LogicExpression, self.__node, "^", other)

    def __eq__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ComparisonExpression, self.__node, "==", other)

    def __ne__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ComparisonExpression, self.__node, "!=", other)

    def __lt__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ComparisonExpression, self.__node, "<", other)

    def __le__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ComparisonExpression, self.__node, "<=", other)

    def __gt__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ComparisonExpression, self.__node, ">", other)

    def __ge__(self, other: Value) -> InteractiveNode:
        return _binary_expr(ComparisonExpression, self.__node, ">=", other)

    def __neg__(self) -> InteractiveNode:
        right = InteractiveExpression(self.__node)
        expr = UnaryExpression(Token("-"), right)
        return InteractiveNode(expr.evaluate())

    def __invert__(self) -> InteractiveNode:
        right = InteractiveExpression(self.__node)
        expr = UnaryExpression(Token("!"), right)
        return InteractiveNode(expr.evaluate())

    def __getitem__(self, index: int) -> InteractiveNode:
        left = InteractiveExpression(self.__node)
        indexer = InteractiveExpression(index)
        expr = IndexingExpression(left, indexer)
        return InteractiveNode(expr.evaluate())

    def __getattr__(self, property_: str) -> InteractiveNode:
        # TODO swizzles
        raise NotImplementedError()


def _binary_expr(expr_type: Type, left: Value, op: str, right: Value) -> InteractiveNode:
    left = InteractiveExpression(left)
    right = InteractiveExpression(right)
    expr = expr_type(left, Token(op), right)
    return InteractiveNode(expr.evaluate())
