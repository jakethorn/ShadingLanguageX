from . import Expression
from .. import mtlx
from ..CompileError import CompileError
from ..Keyword import DataType, Keyword, FILENAME, STRING, FLOAT, INTEGER, BOOLEAN
from ..Token import Token
from ..token_types import INT_LITERAL, FLOAT_LITERAL, FILENAME_LITERAL, STRING_LITERAL


class LiteralExpression(Expression):
    def __init__(self, literal: Token):
        super().__init__(literal)
        self.__literal = literal
        self.__null_type: DataType | None = None

    def _init(self, valid_types: list[DataType]) -> None:
        if self.__literal.type == Keyword.NULL and len(valid_types) > 1:
            raise CompileError(f"null type is ambiguous.", self.token)
        self.__null_type = valid_types[0]

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

    def _evaluate(self) -> mtlx.Node:
        if self.__literal.type == Keyword.NULL:
            return mtlx.get_null_node(self.__null_type)
        else:
            return mtlx.constant(self.__literal.value)
