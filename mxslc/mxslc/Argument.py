from __future__ import annotations

from . import mx_utils
from .DataType import DataType
from .Expressions import Expression
from .Token import Token


class Argument:
    """
    Represents a positional or named argument to a function, constructor or node constructor.
    """
    def __init__(self, expr: Expression, position: int, identifier: Token = None):
        self.__expr = expr
        self.__position = position
        self.__identifier = identifier

    @property
    def position(self) -> int:
        return self.__position

    @property
    def name(self) -> str | None:
        return self.__identifier.lexeme if self.__identifier else None

    @property
    def data_type(self) -> DataType:
        return self.__expr.data_type

    @property
    def is_positional(self) -> bool:
        return self.__identifier is None

    @property
    def is_named(self) -> bool:
        return self.__identifier is not None

    @property
    def expression(self) -> Expression:
        return self.__expr

    def instantiate_templated_types(self, template_type: DataType) -> Argument:
        return Argument(self.__expr.instantiate_templated_types(template_type), self.position, self.__identifier)

    def init(self, valid_types: DataType | set[DataType] = None) -> None:
        self.__expr.init(valid_types)

    def evaluate(self) -> mx_utils.Node:
        return self.__expr.evaluate()
