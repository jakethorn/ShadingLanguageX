from abc import ABC

from . import Expression
from .. import mx_utils, utils
from ..CompileError import CompileError
from ..DataType import DataType, INTEGER, FLOAT, BOOLEAN, MULTI_ELEM_TYPES
from ..Keyword import Keyword
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

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        # TODO clean this up
        if set(valid_types) == {INTEGER, FLOAT}:
            self.left.init(valid_types)
            self.right.init(self.left.data_type)
        elif len(valid_types) == 1 and list(valid_types)[0] in [INTEGER, FLOAT]:
            self.left.init(valid_types)
            self.right.init(valid_types)
        elif len(valid_types) == 1 and list(valid_types)[0] in MULTI_ELEM_TYPES:
            left_error = None
            try:
                self.left.init({FLOAT} | valid_types)
            except CompileError as e:
                left_error = e
            right_error = None
            try:
                self.right.init({FLOAT} | valid_types)
            except CompileError as e:
                right_error = e
            if left_error and right_error:
                raise left_error
            elif left_error:
                if self.right.data_type == FLOAT:
                    self.left.init(valid_types)
                else:
                    raise left_error
            elif right_error:
                if self.left.data_type == FLOAT:
                    self.right.init(valid_types)
                else:
                    raise right_error
        elif any(t in MULTI_ELEM_TYPES for t in valid_types):
            self.left.init({FLOAT} | valid_types)
            self.right.init({FLOAT} | valid_types)
        else:
            raise CompileError(f"{self.node_type} operator cannot be evaluated to a {utils.types_string(list(valid_types))}.", self.token)

    def _init(self, valid_types: set[DataType]) -> None:
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

    def _evaluate(self) -> mx_utils.Node:
        left_node = self.left.evaluate()
        right_node = self.right.evaluate()

        if left_node.data_size < right_node.data_size:
            left_node = mx_utils.convert(left_node, right_node.data_type)

        node = mx_utils.create_node(self.node_type, self.data_type)
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

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        if self.operator.type in ["!=", "=="]:
            valid_sub_types = {BOOLEAN, INTEGER, FLOAT}
        else:
            valid_sub_types = {INTEGER, FLOAT}

        left_error = _try_init(self.left, valid_sub_types)
        right_error = _try_init(self.right, valid_sub_types)
        if left_error and right_error:
            raise left_error
        elif left_error:
            self.left.init(self.right.data_type)
        elif right_error:
            self.right.init(self.left.data_type)

        if self.left.data_type != self.right.data_type:
            raise CompileError(f"Cannot compare a {self.left.data_type} and a {self.right.data_type}.", self.token)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> mx_utils.Node:
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

        comp_node = mx_utils.create_node(node_type, BOOLEAN)
        comp_node.set_input("value1", left_node)
        comp_node.set_input("value2", right_node)

        if node_type == "!=":
            bang_node = mx_utils.create_node("not", BOOLEAN)
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

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.left.init(BOOLEAN)
        self.right.init(BOOLEAN)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> mx_utils.Node:
        node_type = {
            "&": "and",
            Keyword.AND: "and",
            "|": "or",
            Keyword.OR: "or"
        }[self.operator.type]

        node = mx_utils.create_node(node_type, BOOLEAN)
        node.set_input("in1", self.left.evaluate())
        node.set_input("in2", self.right.evaluate())
        return node


def _try_init(expr: Expression, valid_types: set[DataType]) -> Exception:
    error = None
    try:
        expr.init(valid_types)
    except CompileError as e:
        error = e
    return error
