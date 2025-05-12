import MaterialX as mx

from .. import mtlx
from ..Expressions import Expression
from ..Keyword import DataType


class InteractiveExpression(Expression):
    def __init__(self, value: mtlx.Value):
        super().__init__(-1)
        if isinstance(value, mx.Node):
            self.__node = mtlx.Node(value)
        else:
            self.__node = mtlx.constant(value)

    @property
    def data_type(self) -> DataType:
        return self.__node.data_type

    def create_node(self) -> mtlx.Node:
        return self.__node
