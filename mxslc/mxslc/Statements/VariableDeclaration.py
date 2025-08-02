from . import Statement
from .. import state
from ..DataType import DataType
from ..Expressions import Expression
from ..Interactive.ValueExpression import ValueExpression
from ..Keyword import Keyword
from ..Token import Token


class VariableDeclaration(Statement):
    def __init__(self, modifiers: list[Token], data_type: Token | DataType, identifier: Token, right: Expression | None):
        super().__init__()
        self.__modifiers = modifiers
        self.__is_global = Keyword.GLOBAL in modifiers
        self.__is_const = Keyword.CONST in modifiers
        self.__data_type = DataType(data_type)
        self.__identifier = identifier
        self.__right = right
        assert self.__is_global == (self.__right is None)

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        data_type = self.__data_type.instantiate(template_type)
        right = self.__right.instantiate_templated_types(template_type)
        return VariableDeclaration(self.__modifiers, data_type, self.__identifier, right)

    def execute(self) -> None:
        if self.__is_global:
            value = state.get_global(self.__identifier)
            self.__right = ValueExpression(value)
        node = self.__right.init_evaluate(self.__data_type)
        state.add_node(self.__identifier, node, self.__is_const)
        self._add_attributes_to_node(node)

    def __str__(self) -> str:
        return f"{self.__data_type} {self.__identifier} = {self.__right};"
