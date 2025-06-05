from . import Statement
from ..Expressions import Expression


class ExpressionStatement(Statement):
    def __init__(self, expr: Expression):
        self.__expr = expr

    def execute(self) -> None:
        self.__expr.init_evaluate()
