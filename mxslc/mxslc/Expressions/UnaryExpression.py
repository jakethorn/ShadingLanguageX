from . import Expression
from .. import mtlx
from ..DataType import DataType, BOOLEAN, NUMERIC_TYPES
from ..Keyword import Keyword
from ..Token import Token


class UnaryExpression(Expression):
    """
    Examples:
        bool b = !a;
        bool c = not b;
        float neg_pi = -3.14;
        vec3 v = vec3(-1.0, +1.0, -1.0);
    """
    def __init__(self, op: Token, right: Expression):
        super().__init__(op)
        self.__op = op
        self.__right = right

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        right = self.__right.instantiate_templated_types(data_type)
        return UnaryExpression(self.__op, right)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        valid_sub_types = BOOLEAN if self.__op in ["!", Keyword.NOT] else NUMERIC_TYPES
        self.__right.init(valid_sub_types)

    @property
    def _data_type(self) -> DataType:
        return self.__right.data_type

    def _evaluate(self) -> mtlx.Node:
        if self.__op in ["!", Keyword.NOT]:
            node = mtlx.create_node("not", BOOLEAN)
            node.set_input("in", self.__right.evaluate())
            return node
        elif self.__op == "-":
            right_node = self.__right.evaluate()
            node = mtlx.create_node("subtract", right_node.data_type)
            node.set_input("in1", right_node.data_type.zeros())
            node.set_input("in2", right_node)
            return node
        else:
            return self.__right.evaluate()
