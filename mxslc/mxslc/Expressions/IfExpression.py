from . import Expression
from .. import mx_utils
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

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        if self.otherwise is None:
            raise CompileError("No else branch provided in if expression", self.token)

        self.clause.init(BOOLEAN)

        then_error = _try_init(self.then, valid_types)
        otherwise_error = _try_init(self.otherwise, valid_types)
        if then_error and otherwise_error:
            raise then_error
        elif then_error:
            self.then.init(self.otherwise.data_type)
        elif otherwise_error:
            self.otherwise.init(self.then.data_type)

        if self.then.data_type != self.otherwise.data_type:
            raise CompileError(f"Branches must be of same data type, but were {self.then.data_type} and {self.otherwise.data_type}.", self.token)

    @property
    def _data_type(self) -> DataType:
        return self.then.data_type

    def _evaluate(self) -> mx_utils.Node:
        clause_node = self.clause.evaluate()
        then_node = self.then.evaluate()
        otherwise_node = self.otherwise.evaluate()

        node = mx_utils.create_node("ifequal", self.data_type)
        node.set_input("value1", clause_node)
        node.set_input("value2", True)
        node.set_input("in1", then_node)
        node.set_input("in2", otherwise_node)

        return node


def _try_init(expr: Expression, valid_types: set[DataType]) -> Exception:
    error = None
    try:
        expr.init(valid_types)
    except CompileError as e:
        error = e
    return error
