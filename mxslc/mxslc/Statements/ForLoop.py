from . import Statement
from .. import state
from ..Argument import Argument
from ..CompileError import CompileError
from ..DataType import DataType
from ..Expressions import Expression, ValueExpression
from ..Expressions.LiteralExpression import NullExpression
from ..Function import create_function
from ..Keyword import Keyword
from ..Parameter import ParameterList, Parameter
from ..Token import Token, IdentifierToken


class ForLoop(Statement):
    def __init__(self,
                 is_inline: bool,
                 iter_var_type: Token | DataType,
                 identifier: Token,
                 start_value: Expression,
                 value2: Expression,
                 value3: Expression | None,
                 body: list[Statement]):
        super().__init__()
        self.__is_inline = is_inline
        self.__iter_var_type = DataType(iter_var_type)
        self.__identifier = identifier
        self.__start_value = start_value
        self.__value2 = value2
        self.__value3 = value3
        self.__body = body

        return_type = DataType(Keyword.VOID)
        func_identifier = IdentifierToken(f"__loop__{state.get_loop_id()}")
        parameters = ParameterList([Parameter(self.__identifier, self.__iter_var_type)])
        return_expr = NullExpression()
        self.__function = create_function(is_inline, return_type, func_identifier, None, parameters, self.__body, return_expr)

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        iter_var_type = self.__iter_var_type.instantiate(template_type)
        stmts = [s.instantiate_templated_types(template_type) for s in self.__body]
        return ForLoop(self.__is_inline, iter_var_type, self.__identifier, self.__start_value, self.__value2, self.__value3, stmts)

    def execute(self) -> None:
        self.__function.initialise()

        self.__start_value.init(self.__iter_var_type)
        self.__value2.init(self.__iter_var_type)
        if self.__value3:
            self.__value3.init(self.__iter_var_type)

        if not (self.__start_value.has_value and self.__value2.has_value and (self.__value3 is None or self.__value3.has_value)):
            raise CompileError("Invalid loop constraint expression.", self.__identifier)

        start_value = self.__start_value.value
        incr_value = self.__value2.value if self.__value3 else self.__iter_var_type.from_value(1)
        end_value = self.__value3.value if self.__value3 else self.__value2.value

        i = start_value
        while i <= end_value:
            iter_arg = Argument(ValueExpression(i), 0)
            iter_arg.init(self.__iter_var_type)
            self.__function.invoke([iter_arg])
            i += incr_value
