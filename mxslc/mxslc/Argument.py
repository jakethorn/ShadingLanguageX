from __future__ import annotations

from .DataType import DataType
from .Expressions import Expression, ExpressionFacade
from .Token import Token


class Argument(ExpressionFacade):
    """
    Represents a positional or named argument to a function, constructor or node constructor.
    """
    def __init__(self, expr: Expression, position: int, identifier: Token = None):
        if identifier:
            str_ = f"{identifier.lexeme}={expr}"
        else:
            str_ = f"{expr}"

        super().__init__(expr, str_)
        self.__position = position
        self.__identifier = identifier

    @property
    def position(self) -> int:
        return self.__position

    @property
    def name(self) -> str | None:
        return self.__identifier.lexeme if self.__identifier else None

    @property
    def is_positional(self) -> bool:
        return self.__identifier is None

    @property
    def is_named(self) -> bool:
        return self.__identifier is not None

    def instantiate_templated_types(self, template_type: DataType) -> Argument:
        return Argument(self.instantiate_templated_types(template_type), self.position, self.__identifier)
