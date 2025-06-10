from . import Statement
from .. import state, mx_utils
from ..CompileError import CompileError
from ..DataType import DataType, FLOAT
from ..Token import Token
from ..token_types import FLOAT_LITERAL


class ForLoop(Statement):
    NAMESPACE = "<loop>"

    def __init__(self, iter_var_type: Token | DataType, identifier: Token, start_value: Token, value2: Token, value3: Token | None, statements: list[Statement]):
        self.__iter_var_type = DataType(iter_var_type)
        self.__identifier = identifier
        self.__start_value = start_value
        self.__value2 = value2
        self.__value3 = value3
        self.__statements = statements

        if self.__iter_var_type != FLOAT:
            raise CompileError("Loop iteration variable must be a float.", self.__identifier)

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        iter_var_type = self.__iter_var_type.instantiate(template_type)
        statements = [s.instantiate_templated_types(template_type) for s in self.__statements]
        return ForLoop(iter_var_type, self.__identifier, self.__start_value, self.__value2, self.__value3, statements)

    def execute(self) -> None:
        start_value = _get_loop_value(self.__start_value)
        incr_value = _get_loop_value(self.__value2) if self.__value3 else 1.0
        end_value = _get_loop_value(self.__value3 or self.__value2)

        i = start_value
        while i <= end_value:
            state.enter_scope(self.NAMESPACE)
            state.add_node(self.__identifier, mx_utils.constant(i))
            for statement in self.__statements:
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
            raise CompileError("For loop variables can only be literals or constant values.", token)
