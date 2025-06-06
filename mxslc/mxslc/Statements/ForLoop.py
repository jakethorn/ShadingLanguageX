from . import Statement
from .. import state, mtlx
from ..CompileError import CompileError
from ..Keyword import DataType
from ..Token import Token
from ..token_types import FLOAT_LITERAL


class ForLoop(Statement):
    NAMESPACE = "<loop>"

    def __init__(self, data_type: Token, identifier: Token, start_value: Token, value2: Token, value3: Token, statements: list[Statement]):
        self.data_type = data_type
        self.identifier = identifier
        self.start_value = start_value
        self.value2 = value2
        self.value3 = value3
        self.statements = statements

    def instantiate_templated_types(self, data_type: DataType) -> Statement:
        data_type_token = Token(data_type) if DataType(self.data_type.type) is DataType.T else self.data_type
        statements = [s.instantiate_templated_types(data_type) for s in self.statements]
        return ForLoop(data_type_token, self.identifier, self.start_value, self.value2, self.value3, statements)

    def execute(self) -> None:
        start_value = _get_loop_value(self.start_value)
        incr_value = _get_loop_value(self.value2) if self.value3 is not None else 1.0
        end_value = _get_loop_value(self.value3 or self.value2)

        i = start_value
        while i <= end_value:
            state.enter_scope(self.NAMESPACE)
            state.add_node(self.identifier, mtlx.constant(i))
            for statement in self.statements:
                statement.execute()
            state.exit_scope()
            i += incr_value


def _get_loop_value(token: Token) -> float:
    if token == FLOAT_LITERAL:
        return token.value
    else:
        node = state.get_node(token)
        if node.category == "constant":
            return node.get_input("value")
        else:
            raise CompileError("For loop variables can only be float literals or constant values.", token)
