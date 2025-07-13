from abc import ABC

from . import Expression
from .expression_utils import init_linked_expressions
from .. import utils, node_utils
from ..CompileError import CompileError
from ..DataType import DataType, INTEGER, FLOAT, BOOLEAN, MULTI_ELEM_TYPES
from ..Keyword import Keyword
from ..mx_wrapper import Node
from ..Token import Token
from ..utils import one


class BinaryExpression(Expression, ABC):
    def __init__(self, left: Expression, op: Token, right: Expression):
        super().__init__(op)
        self._left = left
        self._op = op
        self._right = right

    def __str__(self) -> str:
        return f"{self._left} {self._op} {self._right}"


class ArithmeticExpression(BinaryExpression):
    def __init__(self, left: Expression, op: Token, right: Expression):
        super().__init__(left, op, right)
        self.node_type = {
            "+": "add",
            "-": "subtract",
            "*": "multiply",
            "/": "divide",
            "%": "modulo",
            "^": "power"
        }[self._op.type]

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        left = self._left.instantiate_templated_types(template_type)
        right = self._right.instantiate_templated_types(template_type)
        return ArithmeticExpression(left, self._op, right)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        # TODO clean this up
        if set(valid_types) == {INTEGER, FLOAT}:
            self._left.init(valid_types)
            self._right.init(self._left.data_type)
        elif len(valid_types) == 1 and list(valid_types)[0] in [INTEGER, FLOAT]:
            self._left.init(valid_types)
            self._right.init(valid_types)
        elif len(valid_types) == 1 and list(valid_types)[0] in MULTI_ELEM_TYPES:
            left_error = None
            try:
                self._left.init({FLOAT} | valid_types)
            except CompileError as e:
                left_error = e
            right_error = None
            try:
                self._right.init({FLOAT} | valid_types)
            except CompileError as e:
                right_error = e
            if left_error and right_error:
                raise left_error
            elif left_error:
                if self._right.data_type == FLOAT:
                    self._left.init(valid_types)
                else:
                    raise left_error
            elif right_error:
                if self._left.data_type == FLOAT:
                    self._right.init(valid_types)
                else:
                    raise right_error
        elif any(t in MULTI_ELEM_TYPES for t in valid_types):
            self._left.init({FLOAT} | valid_types)
            self._right.init({FLOAT} | valid_types)
        else:
            raise CompileError(f"{self.node_type} operator cannot be evaluated to a {utils.format_types(valid_types)}.", self.token)

    def _init(self, valid_types: set[DataType]) -> None:
        if one(e.data_type == INTEGER for e in [self._left, self._right]):
            raise CompileError("Integers cannot be combined with other types.", self.token)
        if all(e.data_size > 1 for e in [self._left, self._right]) and self._left.data_type != self._right.data_type:
            raise CompileError(f"Cannot {self.node_type} a {self._left.data_type} and a {self._right.data_type}.", self.token)

    @property
    def _data_type(self) -> DataType:
        if self._left.data_size > self._right.data_size:
            return self._left.data_type
        else:
            return self._right.data_type

    def _evaluate(self) -> Node:
        left_node = self._left.evaluate()
        right_node = self._right.evaluate()

        if left_node.data_size < right_node.data_size:
            left_node = node_utils.convert(left_node, right_node.data_type)

        node = node_utils.create(self.node_type, self.data_type)
        node.set_input("in1", left_node)
        node.set_input("in2", right_node)
        return node


class ComparisonExpression(BinaryExpression):
    def __init__(self, left: Expression, op: Token, right: Expression):
        super().__init__(left, op, right)

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        left = self._left.instantiate_templated_types(template_type)
        right = self._right.instantiate_templated_types(template_type)
        return ComparisonExpression(left, self._op, right)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        if self._op.type in ["!=", "=="]:
            valid_sub_types = {BOOLEAN, INTEGER, FLOAT}
        else:
            valid_sub_types = {INTEGER, FLOAT}
        init_linked_expressions(self._left, self._right, valid_sub_types)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> Node:
        node_type = {
            "!=": "ifequal",
            "==": "ifequal",
            ">": "ifgreater",
            "<": "ifgreatereq",
            ">=": "ifgreatereq",
            "<=": "ifgreater"
        }[self._op.type]

        left_node = self._left.evaluate()
        right_node = self._right.evaluate()

        if self._op in ["<", "<="]:
            left_node, right_node = right_node, left_node

        comp_node = node_utils.create(node_type, BOOLEAN)
        comp_node.set_input("value1", left_node)
        comp_node.set_input("value2", right_node)

        if node_type == "!=":
            bang_node = node_utils.create("not", BOOLEAN)
            bang_node.set_input("in", comp_node)
            return bang_node
        else:
            return comp_node


class LogicExpression(BinaryExpression):
    def __init__(self, left: Expression, op: Token, right: Expression):
        super().__init__(left, op, right)

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        left = self._left.instantiate_templated_types(template_type)
        right = self._right.instantiate_templated_types(template_type)
        return LogicExpression(left, self._op, right)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self._left.init(BOOLEAN)
        self._right.init(BOOLEAN)

    @property
    def _data_type(self) -> DataType:
        return BOOLEAN

    def _evaluate(self) -> Node:
        node_type = {
            "&": "and",
            Keyword.AND: "and",
            "|": "or",
            Keyword.OR: "or"
        }[self._op.type]

        node = node_utils.create(node_type, BOOLEAN)
        node.set_input("in1", self._left.evaluate())
        node.set_input("in2", self._right.evaluate())
        return node
