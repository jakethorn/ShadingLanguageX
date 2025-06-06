from abc import ABC

from . import Expression
from .. import mtlx, utils
from ..CompileError import CompileError
from ..Keyword import DataType, INTEGER, BOOLEAN, FLOAT, Keyword, VECTOR_TYPES, COLOR_TYPES
from ..Token import Token
from ..utils import one


class BinaryExpression(Expression, ABC):
    def __init__(self, left: Expression, operator: Token, right: Expression):
        super().__init__(operator)
        self.left = left
        self.operator = operator
        self.right = right


class ArithmeticExpression(BinaryExpression):
    def __init__(self, left: Expression, operator: Token, right: Expression):
        super().__init__(left, operator, right)
        self.node_type = {
            "+": "add",
            "-": "subtract",
            "*": "multiply",
            "/": "divide",
            "%": "modulo",
            "^": "power"
        }[self.operator.type]

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        left = self.left.instantiate_templated_types(data_type)
        right = self.right.instantiate_templated_types(data_type)
        return ArithmeticExpression(left, self.operator, right)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        if set(valid_types) == {DataType.INTEGER, DataType.FLOAT}:
            self.left.init(valid_types)
            self.right.init(self.left.data_type)
        elif len(valid_types) == 1 and valid_types[0] in [DataType.INTEGER, DataType.FLOAT]:
            self.left.init(valid_types)
            self.right.init(valid_types)
        elif len(valid_types) == 1 and valid_types[0] in VECTOR_TYPES + COLOR_TYPES:
            ambiguous_left = False
            try:
                self.left.init([DataType.FLOAT, *valid_types])
            except CompileError as left_error:
                ambiguous_left = True
            ambiguous_right = False
            try:
                self.right.init([DataType.FLOAT, *valid_types])
            except CompileError as right_error:
                ambiguous_right = True
            if ambiguous_left and ambiguous_right:
                raise left_error
            elif ambiguous_left:
                if self.right.data_type is DataType.FLOAT:
                    self.left.init(valid_types)
                else:
                    raise left_error
            elif ambiguous_right:
                if self.left.data_type is DataType.FLOAT:
                    self.right.init(valid_types)
                else:
                    raise right_error
        elif any(t in VECTOR_TYPES + COLOR_TYPES for t in valid_types):
            self.left.init([DataType.FLOAT, *valid_types])
            self.right.init([DataType.FLOAT, *valid_types])
        else:
            raise CompileError(f"{self.node_type} operator cannot be evaluated to a {utils.types_string(valid_types)}.", self.token)

    def _init(self, valid_types: list[DataType]) -> None:
        if one(e.data_type == INTEGER for e in [self.left, self.right]):
            raise CompileError("Integers cannot be combined with other types.", self.token)
        if all(e.data_size > 1 for e in [self.left, self.right]) and self.left.data_type != self.right.data_type:
            raise CompileError(f"Cannot {self.node_type} a {self.left.data_type} and a {self.right.data_type}.", self.token)

    @property
    def _data_type(self) -> DataType:
        if self.left.data_size > self.right.data_size:
            return self.left.data_type
        else:
            return self.right.data_type

    def _evaluate(self) -> mtlx.Node:
        left_node = self.left.evaluate()
        right_node = self.right.evaluate()

        if left_node.data_size < right_node.data_size:
            left_node = mtlx.convert(left_node, right_node.data_type)

        node = mtlx.create_node(self.node_type, self.data_type)
        node.set_input("in1", left_node)
        node.set_input("in2", right_node)
        return node


class ComparisonExpression(BinaryExpression):
    def __init__(self, left: Expression, operator: Token, right: Expression):
        super().__init__(left, operator, right)

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        left = self.left.instantiate_templated_types(data_type)
        right = self.right.instantiate_templated_types(data_type)
        return ComparisonExpression(left, self.operator, right)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.left.init([BOOLEAN, INTEGER, FLOAT])
        self.right.init([BOOLEAN, INTEGER, FLOAT])

    def _init(self, valid_types: list[DataType]) -> None:
        if self.left.data_type != self.right.data_type:
            raise CompileError(f"Cannot compare a {self.left.data_type} and a {self.right.data_type}.", self.token)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> mtlx.Node:
        node_type = {
            "!=": "ifequal",
            "==": "ifequal",
            ">": "ifgreater",
            "<": "ifgreatereq",
            ">=": "ifgreatereq",
            "<=": "ifgreater"
        }[self.operator.type]

        left_node = self.left.evaluate()
        right_node = self.right.evaluate()

        if self.operator in ["<", "<="]:
            left_node, right_node = right_node, left_node

        comp_node = mtlx.create_node(node_type, BOOLEAN)
        comp_node.set_input("value1", left_node)
        comp_node.set_input("value2", right_node)

        if node_type == "!=":
            bang_node = mtlx.create_node("not", BOOLEAN)
            bang_node.set_input("in", comp_node)
            return bang_node
        else:
            return comp_node


class LogicExpression(BinaryExpression):
    def __init__(self, left: Expression, operator: Token, right: Expression):
        super().__init__(left, operator, right)

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        left = self.left.instantiate_templated_types(data_type)
        right = self.right.instantiate_templated_types(data_type)
        return LogicExpression(left, self.operator, right)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.left.init(BOOLEAN)
        self.right.init(BOOLEAN)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> mtlx.Node:
        node_type = {
            "&": "and",
            Keyword.AND: "and",
            "|": "or",
            Keyword.OR: "or"
        }[self.operator.type]

        node = mtlx.create_node(node_type, BOOLEAN)
        node.set_input("in1", self.left.evaluate())
        node.set_input("in2", self.right.evaluate())
        return node
