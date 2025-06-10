import re

from . import Expression
from .. import mx_utils
from ..CompileError import CompileError
from ..DataType import DataType, VECTOR2, VECTOR3, VECTOR4, COLOR4, COLOR3
from ..Token import Token
from ..utils import type_of_swizzle, string


class SwizzleExpression(Expression):
    def __init__(self, left: Expression, swizzle: Token):
        super().__init__(swizzle)
        self.__left = left
        self.__swizzle = string(swizzle)

        if not re.fullmatch(r"([xyzw]{1,4}|[rgba]{1,4})", self.__swizzle):
            raise CompileError(f"'{self.__swizzle}' is not a valid swizzle.", self.token)

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        left = self.__left.instantiate_templated_types(template_type)
        return SwizzleExpression(left, self.token)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.__left.init(self.__valid_left_types())

    @property
    def _data_type(self) -> DataType:
        return type_of_swizzle(self.__swizzle)

    def _evaluate(self) -> mx_utils.Node:
        left_node = self.__left.evaluate()
        if len(self.__swizzle) == 1:
            return mx_utils.extract(left_node, self.__swizzle)
        else:
            channels = [mx_utils.extract(left_node, c) for c in self.__swizzle]
            return mx_utils.combine(channels, self.data_type)

    def __valid_left_types(self) -> set[DataType]:
        if "x" in self.__swizzle:
            return {VECTOR2, VECTOR3, VECTOR4}
        if "y" in self.__swizzle:
            return {VECTOR2, VECTOR3, VECTOR4}
        if "z" in self.__swizzle:
            return {VECTOR3, VECTOR4}
        if "w" in self.__swizzle:
            return {VECTOR4}
        if "r" in self.__swizzle:
            return {COLOR3, COLOR4}
        if "g" in self.__swizzle:
            return {COLOR3, COLOR4}
        if "b" in self.__swizzle:
            return {COLOR3, COLOR4}
        if "a" in self.__swizzle:
            return {COLOR4}
        raise CompileError(f"'{self.__swizzle}' is not a valid swizzle.", self.token)
