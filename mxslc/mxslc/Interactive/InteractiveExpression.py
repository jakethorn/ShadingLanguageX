import MaterialX as mx

from .. import mtlx
from ..Expressions import Expression
from ..Keyword import DataType


class InteractiveExpression(Expression):
    def __init__(self, value: mtlx.Value):
        super().__init__(None)
        if isinstance(value, mx.Node):
            self.__node = mtlx.Node(value)
        else:
            self.__node = mtlx.constant(value)

    @property
    def _data_type(self) -> DataType:
        return self.__node.data_type

    def _evaluate(self) -> mtlx.Node:
        return self.__node
