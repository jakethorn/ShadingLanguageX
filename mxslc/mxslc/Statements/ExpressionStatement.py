from . import Statement
from ..DataType import DataType
from ..Expressions import Expression


class ExpressionStatement(Statement):
    def __init__(self, expr: Expression):
        self.__expr = expr

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        expr = self.__expr.instantiate_templated_types(template_type)
        return ExpressionStatement(expr)

    def execute(self) -> None:
        self.__expr.init_evaluate()
