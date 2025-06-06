from . import Expression
from .. import state, mtlx
from ..Keyword import DataType
from ..Token import Token


class IdentifierExpression(Expression):
    def __init__(self, identifier: Token):
        super().__init__(identifier)
        self.__identifier = identifier

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        return self

    @property
    def _data_type(self) -> DataType:
        node = state.get_node(self.__identifier)
        return node.data_type

    def _evaluate(self) -> mtlx.Node:
        old_node = state.get_node(self.__identifier)
        new_node = mtlx.create_node("dot", self.data_type)
        new_node.set_input("in", old_node)
        return new_node
