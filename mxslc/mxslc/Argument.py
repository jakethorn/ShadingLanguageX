from __future__ import annotations

from . import mtlx
from .DataType import DataType
from .Expressions import Expression
from .Token import Token


# TODO add position index (maybe to parameter as well) then remove all enumate(args) code
# TODO maybe this can inherit from Expression
class Argument:
    """
    Represents a positional or named argument to a function, constructor or standard library call.
    """
    def __init__(self, expr: Expression, name: Token = None):
        self.__expr = expr
        self.__name = name.lexeme if name is not None else None
        self.__name_token = name

    @property
    def name(self) -> str:
        return self.__name

    @property
    def data_type(self) -> DataType:
        return self.__expr.data_type

    @property
    def is_positional(self) -> bool:
        return self.__name is None

    @property
    def is_named(self) -> bool:
        return self.__name is not None

    @property
    def expression(self) -> Expression:
        return self.__expr

    def instantiate_templated_types(self, data_type: DataType) -> Argument:
        return Argument(self.__expr.instantiate_templated_types(data_type), self.__name_token)

    def evaluate(self) -> mtlx.Node:
        return self.__expr.evaluate()
