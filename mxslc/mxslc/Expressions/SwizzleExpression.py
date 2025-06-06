import re

from . import Expression
from .. import mtlx
from ..CompileError import CompileError
from ..Keyword import DataType
from ..Token import Token
from ..token_types import IDENTIFIER
from ..utils import type_of_swizzle


class SwizzleExpression(Expression):
    def __init__(self, left: Expression, swizzle: Token):
        super().__init__(swizzle)
        self.left = left
        self.swizzle = swizzle.lexeme

        if not re.fullmatch(r"([xyzw]{1,4}|[rgba]{1,4})", self.swizzle):
            raise CompileError(f"'{self.swizzle}' is not a valid swizzle.", self.token)

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        left = self.left.instantiate_templated_types(data_type)
        return SwizzleExpression(left, Token(IDENTIFIER, self.swizzle))

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.left.init(self.__valid_left_types())

    @property
    def _data_type(self) -> DataType:
        return type_of_swizzle(self.swizzle)

    def _evaluate(self) -> mtlx.Node:
        left_node = self.left.evaluate()
        if len(self.swizzle) == 1:
            return mtlx.extract(left_node, self.swizzle)
        else:
            channels = [mtlx.extract(left_node, c) for c in self.swizzle]
            return mtlx.combine(channels, self.data_type)

    def __valid_left_types(self) -> list[DataType]:
        if "x" in self.swizzle:
            return [DataType.VECTOR2, DataType.VECTOR3, DataType.VECTOR4]
        if "y" in self.swizzle:
            return [DataType.VECTOR2, DataType.VECTOR3, DataType.VECTOR4]
        if "z" in self.swizzle:
            return [DataType.VECTOR3, DataType.VECTOR4]
        if "w" in self.swizzle:
            return [DataType.VECTOR4]
        if "r" in self.swizzle:
            return [DataType.COLOR3, DataType.COLOR4]
        if "g" in self.swizzle:
            return [DataType.COLOR3, DataType.COLOR4]
        if "b" in self.swizzle:
            return [DataType.COLOR3, DataType.COLOR4]
        if "a" in self.swizzle:
            return [DataType.COLOR4]
        raise CompileError(f"'{self.swizzle}' is not a valid swizzle.", self.token)