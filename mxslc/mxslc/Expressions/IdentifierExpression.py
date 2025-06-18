from . import Expression
from .. import state, mx_utils
from ..DataType import DataType
from ..Token import Token


class IdentifierExpression(Expression):
    def __init__(self, identifier: Token):
        super().__init__(identifier)
        self.__identifier = identifier

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        return IdentifierExpression(self.__identifier)

    def _init(self, valid_types: set[DataType]) -> None:
        # raises exception if node is not found
        _ = state.get_node(self.__identifier)

    @property
    def _data_type(self) -> DataType:
        node = state.get_node(self.__identifier)
        return node.data_type

    def _evaluate(self) -> mx_utils.Node:
        old_node = state.get_node(self.__identifier)
        new_node = mx_utils.create_node("dot", self.data_type)
        new_node.set_input("in", old_node)
        return new_node

    def __str__(self) -> str:
        return str(self.__identifier)
