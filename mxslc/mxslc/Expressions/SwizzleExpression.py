import re

from . import Expression
from .. import mx_utils
from ..CompileError import CompileError
from ..DataType import DataType, VECTOR2, VECTOR3, VECTOR4, COLOR4, COLOR3
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

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.left.init(self.__valid_left_types())

    @property
    def _data_type(self) -> DataType:
        return type_of_swizzle(self.swizzle)

    def _evaluate(self) -> mx_utils.Node:
        left_node = self.left.evaluate()
        if len(self.swizzle) == 1:
            return mx_utils.extract(left_node, self.swizzle)
        else:
            channels = [mx_utils.extract(left_node, c) for c in self.swizzle]
            return mx_utils.combine(channels, self.data_type)

    def __valid_left_types(self) -> set[DataType]:
        if "x" in self.swizzle:
            return {VECTOR2, VECTOR3, VECTOR4}
        if "y" in self.swizzle:
            return {VECTOR2, VECTOR3, VECTOR4}
        if "z" in self.swizzle:
            return {VECTOR3, VECTOR4}
        if "w" in self.swizzle:
            return {VECTOR4}
        if "r" in self.swizzle:
            return {COLOR3, COLOR4}
        if "g" in self.swizzle:
            return {COLOR3, COLOR4}
        if "b" in self.swizzle:
            return {COLOR3, COLOR4}
        if "a" in self.swizzle:
            return {COLOR4}
        raise CompileError(f"'{self.swizzle}' is not a valid swizzle.", self.token)
