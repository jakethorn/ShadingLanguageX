from . import Statement
from .. import state
from ..Argument import Argument
from ..CompileError import CompileError
from ..DataType import DataType, FLOAT
from ..Expressions import LiteralExpression
from ..Expressions.LiteralExpression import NullExpression
from ..Function import Function
from ..Keyword import Keyword
from ..Parameter import ParameterList, Parameter
from ..Token import Token, IdentifierToken, LiteralToken
from ..token_types import FLOAT_LITERAL


class ForLoop(Statement):
    def __init__(self, iter_var_type: Token | DataType, identifier: Token, start_value: Token, value2: Token, value3: Token | None, body: list[Statement]):
        self.__iter_var_type = DataType(iter_var_type)
        self.__identifier = identifier
        self.__start_value = start_value
        self.__value2 = value2
        self.__value3 = value3
        self.__body = body

        # TODO support other iter types
        if self.__iter_var_type != FLOAT:
            raise CompileError("Loop iteration variable must be a float.", self.__identifier)

        return_type = DataType(Keyword.VOID)
        func_identifier = IdentifierToken(f"__loop__{state.get_loop_id()}")
        parameters = ParameterList([Parameter(self.__identifier, self.__iter_var_type)])
        return_expr = NullExpression()
        self.__function = Function(return_type, func_identifier, None, parameters, self.__body, return_expr)

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        iter_var_type = self.__iter_var_type.instantiate(template_type)
        stmts = [s.instantiate_templated_types(template_type) for s in self.__body]
        return ForLoop(iter_var_type, self.__identifier, self.__start_value, self.__value2, self.__value3, stmts)

    def execute(self) -> None:
        self.__function.initialise()

        start_value = _get_loop_value(self.__start_value)
        incr_value = _get_loop_value(self.__value2) if self.__value3 else 1.0
        end_value = _get_loop_value(self.__value3 or self.__value2)

        i = start_value
        while i <= end_value:
            iter_arg = Argument(LiteralExpression(LiteralToken(i)), 0)
            iter_arg.init(FLOAT)
            self.__function.invoke([iter_arg])
            i += incr_value


def _get_loop_value(token: Token) -> float:
    if token == FLOAT_LITERAL:
        return token.value
    else:
        node = state.get_node(token)
        if node.category == "constant":
            return node.get_input("value").value
        else:
            raise CompileError("For loop variables can only be literals or constant values.", token)
