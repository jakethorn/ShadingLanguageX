from . import Expression, IdentifierExpression
from .. import node_utils, state
from ..DataType import DataType
from ..Token import Token


class VariableDeclarationExpression(IdentifierExpression):
    def __init__(self, data_type: Token | DataType, identifier: Token):
        super().__init__(identifier)
        self.__data_type = DataType(data_type)
        self.__identifier = identifier

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        return VariableDeclarationExpression(self.__data_type.instantiate(template_type), self.__identifier)

    def _init(self, valid_types: set[DataType]) -> None:
        node = node_utils.constant(self.__data_type.default())
        state.add_node(self.__identifier, node)

    def __str__(self) -> str:
        return f"{self.__data_type} {self.__identifier}"
