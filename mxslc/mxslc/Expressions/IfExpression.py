from . import Expression
from .. import mtlx
from ..CompileError import CompileError
from ..DataType import DataType, BOOLEAN
from ..Token import Token


# TODO implement if else
class IfExpression(Expression):
    def __init__(self, token: Token, clause: Expression, then: Expression, otherwise: Expression):
        super().__init__(token)
        self.clause = clause
        self.then = then
        self.otherwise = otherwise

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        clause = self.clause.instantiate_templated_types(data_type)
        then = self.then.instantiate_templated_types(data_type)
        otherwise = self.otherwise.instantiate_templated_types(data_type)
        return IfExpression(self.token, clause, then, otherwise)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.clause.init(BOOLEAN)
        self.then.init(valid_types)
        self.otherwise.init(valid_types)

    def _init(self, valid_types: list[DataType]) -> None:
        if self.then.data_type != self.otherwise.data_type:
            raise CompileError(f"Branches must be of same data type, but were {self.then.data_type} and {self.otherwise.data_type}.", self.token)

    @property
    def _data_type(self) -> DataType:
        return self.then.data_type

    def _evaluate(self) -> mtlx.Node:
        clause_node = self.clause.evaluate()
        then_node = self.then.evaluate()
        otherwise_node = self.otherwise.evaluate()

        node = mtlx.create_node("ifequal", self.data_type)
        node.set_input("value1", clause_node)
        node.set_input("value2", True)
        node.set_input("in1", then_node)
        node.set_input("in2", otherwise_node)

        return node
