from . import Expression, ComparisonExpression, LogicExpression
from .. import mtlx
from ..Keyword import DataType, BOOLEAN, Keyword
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

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.__and.init(BOOLEAN)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> mtlx.Node:
        return self.__and.evaluate()
