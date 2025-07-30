from . import Statement
from ..DataType import DataType
from ..Expressions import Expression


class ExpressionStatement(Statement):
    def __init__(self, expr: Expression):
        super().__init__()
        self.__expr = expr

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        expr = self.__expr.instantiate_templated_types(template_type)
        return ExpressionStatement(expr)

    def execute(self) -> None:
        node = self.__expr.init_evaluate()
        self._add_attributes_to_node(node)
        if node.is_null_node:
            node.remove()

    def __str__(self) -> str:
        return f"{self.__expr};"
