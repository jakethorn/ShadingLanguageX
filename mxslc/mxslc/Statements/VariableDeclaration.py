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

    def execute(self) -> None:
        node = self.__right.init_evaluate(self.__data_type)
        state.add_node(self.__identifier, node)
