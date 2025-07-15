from . import Expression, BinaryExpression
from ..DataType import DataType, BOOLEAN
from ..Keyword import Keyword
from ..Token import Token
from ..mx_wrapper import Node


class TernaryRelationalExpression(Expression):
    """
    Examples:
        bool is_normalized = -1.0 < x < 1.0;
    """
    def __init__(self, left: Expression, op1: Token, middle: Expression, op2: Token, right: Expression):
        super().__init__(op2)
        comp1 = BinaryExpression(left, op1, middle)
        comp2 = BinaryExpression(middle, op2, right)
        self.__and = BinaryExpression(comp1, Token(Keyword.AND), comp2)

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        return self.__and.instantiate_templated_types(template_type)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.__and.init(BOOLEAN)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> Node:
        return self.__and.evaluate()

    def __str__(self) -> str:
        raise NotImplementedError
