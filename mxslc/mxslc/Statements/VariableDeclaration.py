from . import Statement
from .. import state
from ..Expressions import Expression
from ..Keyword import DataType
from ..Token import Token


class VariableDeclaration(Statement):
    def __init__(self, data_type: Token, identifier: Token, right: Expression):
        self.__data_type = DataType(data_type.type)
        self.__identifier = identifier
        self.__right = right

    def instantiate_templated_types(self, data_type: DataType) -> Statement:
        data_type_token = Token(data_type if self.__data_type is DataType.T else self.__data_type)
        right = self.__right.instantiate_templated_types(data_type)
        return VariableDeclaration(data_type_token, self.__identifier, right)

    def execute(self) -> None:
        node = self.__right.init_evaluate(self.__data_type)
        state.add_node(self.__identifier, node)
