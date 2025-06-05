from . import Expression
from .. import mtlx
from ..Keyword import DataType


class GroupingExpression(Expression):
    def __init__(self, expr: Expression):
        super().__init__(expr.token)
        self.__expr = expr

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.__expr.init(valid_types)

    @property
    def _data_type(self) -> DataType:
        return self.__expr.data_type

    def _evaluate(self) -> mtlx.Node:
        return self.__expr.evaluate()
