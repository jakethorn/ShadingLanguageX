from . import Statement
from ..Expressions import Expression
from ..Keyword import DataType


class ExpressionStatement(Statement):
    def __init__(self, expr: Expression):
        self.__expr = expr

    def instantiate_templated_types(self, data_type: DataType) -> Statement:
        expr = self.__expr.instantiate_templated_types(data_type)
        return ExpressionStatement(expr)

    def execute(self) -> None:
        self.__expr.init_evaluate()
