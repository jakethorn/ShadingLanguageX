from .. import node_utils
from ..DataType import DataType
from ..Expressions import Expression
from ..mx_wrapper import Node, Uniform, type_of


class ValueExpression(Expression):
    def __init__(self, value: Uniform):
        super().__init__(None)
        self.__value = value

    @property
    def _value(self) -> Uniform | None:
        return self.__value

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        return self

    @property
    def _data_type(self) -> DataType:
        return type_of(self.__value)

    def _evaluate(self) -> Node:
        return node_utils.constant(self.__value)
