from . import Expression
from .. import state, mtlx
from ..DataType import DataType
from ..Token import Token


class IdentifierExpression(Expression):
    def __init__(self, identifier: Token):
        super().__init__(identifier)
        self.__identifier = identifier

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        return self

    def _init(self, valid_types: list[DataType]) -> None:
        # raises exception if node is not found
        state.get_node(self.__identifier)

    @property
    def _data_type(self) -> DataType:
        node = state.get_node(self.__identifier)
        return node.data_type

    def _evaluate(self) -> mtlx.Node:
        old_node = state.get_node(self.__identifier)
        new_node = mtlx.create_node("dot", self.data_type)
        new_node.set_input("in", old_node)
        return new_node
