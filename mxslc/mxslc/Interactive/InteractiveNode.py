from __future__ import annotations

import MaterialX as mx

from .InteractiveExpression import InteractiveExpression
from .. import mtlx
from ..Expressions import ArithmeticExpression
from ..Token import Token


class InteractiveNode(mx.Node):
    def __add__(self, other: mx.Node | mtlx.Constant) -> InteractiveNode:
        left = InteractiveExpression(self)
        right = InteractiveExpression(other)
        expr = ArithmeticExpression(left, Token("="), right)
        return make_interactive(mtlx.get_source(expr.evaluate()))


def make_interactive(node: mx.Node) -> InteractiveNode:
    interactive_node = InteractiveNode.__new__(InteractiveNode)
    interactive_node.__dict__.update(node.__dict__)
    return interactive_node
