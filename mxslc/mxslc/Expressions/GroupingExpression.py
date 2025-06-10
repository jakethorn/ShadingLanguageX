from . import Expression
from .. import mx_utils
from ..DataType import DataType


class GroupingExpression(Expression):
    """
    Examples:
        float x = 1.0 * (2.0 + 3.0);
                        ^         ^
    """
    def __init__(self, expr: Expression):
        super().__init__(expr.token)
        self.__expr = expr

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        expr = self.__expr.instantiate_templated_types(template_type)
        return GroupingExpression(expr)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.__expr.init(valid_types)

    @property
    def _data_type(self) -> DataType:
        return self.__expr.data_type

    def _evaluate(self) -> mx_utils.Node:
        return self.__expr.evaluate()
