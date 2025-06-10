from . import Expression, ComparisonExpression, LogicExpression
from .. import mx_utils
from ..DataType import DataType, BOOLEAN
from ..Keyword import Keyword
from ..Token import Token


class TernaryRelationalExpression(Expression):
    """
    Examples:
        bool is_normalized = -1.0 < x < 1.0;
    """
    def __init__(self, left: Expression, op1: Token, middle: Expression, op2: Token, right: Expression):
        super().__init__(op2)
        comp1 = ComparisonExpression(left, op1, middle)
        comp2 = ComparisonExpression(middle, op2, right)
        self.__and = LogicExpression(comp1, Token(Keyword.AND), comp2)

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        return self.__and.instantiate_templated_types(template_type)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.__and.init(BOOLEAN)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> mx_utils.Node:
        return self.__and.evaluate()
