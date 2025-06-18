from . import Expression
from .. import mx_utils
from ..CompileError import CompileError
from ..DataType import DataType, BOOLEAN, INTEGER, FLOAT, STRING, FILENAME
from ..Keyword import Keyword
from ..Token import Token
from ..token_types import INT_LITERAL, FLOAT_LITERAL, FILENAME_LITERAL, STRING_LITERAL


class LiteralExpression(Expression):
    def __init__(self, literal: Token):
        super().__init__(literal)
        self.__literal = literal
        self.__null_type: DataType | None = None

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        return LiteralExpression(self.token)

    def _init(self, valid_types: set[DataType]) -> None:
        if self.__literal.type == Keyword.NULL and len(valid_types) > 1:
            raise CompileError(f"null type is ambiguous.", self.token)
        self.__null_type = list(valid_types)[0]

    @property
    def _data_type(self) -> DataType:
        return {
            Keyword.TRUE: BOOLEAN,
            Keyword.FALSE: BOOLEAN,
            INT_LITERAL: INTEGER,
            FLOAT_LITERAL: FLOAT,
            STRING_LITERAL: STRING,
            FILENAME_LITERAL: FILENAME,
            Keyword.NULL: self.__null_type
        }[self.__literal.type]

    def _evaluate(self) -> mx_utils.Node:
        if self.__literal.type == Keyword.NULL:
            return mx_utils.get_null_node(self.__null_type)
        else:
            return mx_utils.constant(self.__literal.value)

    def __str__(self) -> str:
        return str(self.__literal)


class NullExpression(LiteralExpression):
    def __init__(self):
        super().__init__(Token(Keyword.NULL))
