import MaterialX as mx

from .. import mx_utils
from ..DataType import DataType
from ..Expressions import Expression


class InteractiveExpression(Expression):
    def __init__(self, value: mx_utils.Value):
        super().__init__(None)
        if isinstance(value, mx.Node):
            self.__node = mx_utils.Node(value)
        else:
            self.__node = mx_utils.constant(value)

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        return self

    @property
    def _data_type(self) -> DataType:
        return self.__node.data_type

    def _evaluate(self) -> mx_utils.Node:
        return self.__node
