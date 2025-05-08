from . import Expression
from .. import mtlx
from ..Keyword import DataType


class NodeExpression(Expression):
    def __init__(self, node: mtlx.Node):
        # TODO fix -1
        super().__init__(-1)
        self.__node = node

    @property
    def data_type(self) -> DataType:
        return self.__node.data_type

    def create_node(self) -> mtlx.Node:
        return self.__node


class ConstantExpression(NodeExpression):
    def __init__(self, value: mtlx.Constant):
        node = mtlx.constant(value)
        super().__init__(node)
