from . import Expression
from .. import utils, node_utils
from ..CompileError import CompileError
from ..DataType import DataType, BOOLEAN, INTEGER, FLOAT, MULTI_ELEM_TYPES
from ..Keyword import Keyword
from ..Token import Token
from ..mx_wrapper import Node


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

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        right = self.__right.instantiate_templated_types(template_type)
        return UnaryExpression(self.token, right)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        if self.__op in ["!", Keyword.NOT]:
            valid_sub_types = BOOLEAN
        else:
            valid_sub_types = valid_types & ({INTEGER, FLOAT} | MULTI_ELEM_TYPES)
            if len(valid_sub_types) == 0:
                raise CompileError(f"Invalid data type for unary expression: {utils.format_types(valid_types)}.", self.__op)
        self.__right.init(valid_sub_types)

    @property
    def _data_type(self) -> DataType:
        return self.__right.data_type

    def _evaluate(self) -> Node:
        if self.__op in ["!", Keyword.NOT]:
            node = node_utils.create("not", BOOLEAN)
            node.set_input("in", self.__right.evaluate())
            return node
        elif self.__op == "-":
            right_node = self.__right.evaluate()
            node = node_utils.create("subtract", right_node.data_type)
            node.set_input("in1", right_node.data_type.zeros())
            node.set_input("in2", right_node)
            return node
        else:
            return self.__right.evaluate()

    def __str__(self) -> str:
        return f"{self.__op} {self.__right}"
