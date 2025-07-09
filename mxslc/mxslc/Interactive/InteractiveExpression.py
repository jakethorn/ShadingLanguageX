import MaterialX as mx

from .. import state_utils
from ..DataType import DataType
from ..Expressions import Expression
from ..mx_classes import Node
from ..mx_types import Value


class InteractiveExpression(Expression):
    def __init__(self, value: Value):
        super().__init__(None)
        if isinstance(value, mx.Node):
            self.__node = Node(value)
        else:
            self.__node = state_utils.constant(value)

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        return self

    @property
    def _data_type(self) -> DataType:
        return self.__node.data_type

    def _evaluate(self) -> Node:
        return self.__node
