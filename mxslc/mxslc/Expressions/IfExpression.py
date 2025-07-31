from . import Expression
from .expression_utils import init_linked_expressions
from .. import node_utils
from ..CompileError import CompileError
from ..DataType import DataType, BOOLEAN
from ..mx_wrapper import Node


class IfExpression(Expression):
    def __init__(self, clause: Expression, then: Expression, otherwise: Expression | None):
        super().__init__(clause.token)
        self.__clause = clause
        self.__then = then
        self._otherwise = otherwise

    @property
    def otherwise(self) -> Expression:
        return self._otherwise

    @otherwise.setter
    def otherwise(self, expr: Expression) -> None:
        self._otherwise = expr

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        clause = self.__clause.instantiate_templated_types(template_type)
        then = self.__then.instantiate_templated_types(template_type)
        otherwise = self._otherwise.instantiate_templated_types(template_type)
        return IfExpression(clause, then, otherwise)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        if self._otherwise is None:
            raise CompileError("No else branch provided in if expression", self.token)

        self.__clause.init(BOOLEAN)
        init_linked_expressions(self.__then, self._otherwise, valid_types)

    @property
    def _data_type(self) -> DataType:
        return self.__then.data_type

    def _evaluate(self) -> Node:
        clause_node = self.__clause.evaluate()
        then_node = self.__then.evaluate()
        otherwise_node = self._otherwise.evaluate()

        node = node_utils.create("ifequal", self.data_type)
        node.set_input("value1", clause_node)
        node.set_input("value2", True)
        node.set_input("in1", then_node)
        node.set_input("in2", otherwise_node)

        return node

    def __str__(self) -> str:
        return f"if ({self.__clause}) {{ {self.__then} }} else {{ {self._otherwise} }}"


class IfElseExpression(IfExpression):
    def __init__(self, branches: list[tuple[Expression, Expression]], otherwise: Expression | None):
        self.__final_if_expr = self
        for clause, then in reversed(branches[1:]):
            otherwise = IfExpression(clause, then, otherwise)
            if self.__final_if_expr is self:
                self.__final_if_expr = otherwise
        super().__init__(branches[0][0], branches[0][1], otherwise)

    @property
    def otherwise(self) -> Expression:
        return self.__final_if_expr._otherwise

    @otherwise.setter
    def otherwise(self, expr: Expression) -> None:
        self.__final_if_expr._otherwise = expr
