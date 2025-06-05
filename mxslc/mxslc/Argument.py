from . import mtlx
from .Expressions import Expression
from .Keyword import DataType
from .Token import Token


# TODO add position index (maybe to parameter as well)
# TODO maybe this can inherit from Expression
class Argument:
    """
    Represents a positional or named argument to a function, constructor or standard library call.
    """
    def __init__(self, expr: Expression, name: Token = None):
        self.__expr = expr
        self.__name = name.lexeme if name is not None else None

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

    def evaluate(self) -> mtlx.Node:
        return self.__expr.evaluate()
