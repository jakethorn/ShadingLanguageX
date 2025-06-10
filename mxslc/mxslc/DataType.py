from __future__ import annotations

import MaterialX as mx

from . import mx_utils
from .Keyword import Keyword
from .Token import Token


class DataType:
    """
    Represents a data type (e.g., float, vector3, string, etc...).
    """
    def __new__(cls, data_type: Token | DataType | str, is_templated=False):
        if data_type is None:
            return None
        else:
            return super().__new__(cls)

    def __init__(self, data_type: Token | DataType | str, is_templated=False):
        if isinstance(data_type, Token):
            self.__data_type = data_type.type
        elif isinstance(data_type, DataType):
            self.__data_type = data_type.__data_type
        elif isinstance(data_type, str):
            self.__data_type = data_type
        else:
            raise TypeError
        assert self.__data_type in Keyword.DATA_TYPES()

        if is_templated:
            self.__is_templated = True
        elif isinstance(data_type, DataType):
            self.__is_templated = data_type.__is_templated
        else:
            self.__is_templated = False

    @property
    def is_templated(self):
        return self.__data_type == Keyword.T or self.__is_templated

    def instantiate(self, template_type: DataType | None) -> DataType:
        if self.is_templated and template_type:
            return DataType(template_type, is_templated=True)
        else:
            return self

    @property
    def size(self):
        return {
            Keyword.BOOLEAN: 1,
            Keyword.INTEGER: 1,
            Keyword.FLOAT: 1,
            Keyword.VECTOR2: 2,
            Keyword.VECTOR3: 3,
            Keyword.VECTOR4: 4,
            Keyword.COLOR3: 3,
            Keyword.COLOR4: 4
        }[Keyword(self.__data_type)]

    def zeros(self) -> mx_utils.Constant:
        return {
            Keyword.BOOLEAN: False,
            Keyword.INTEGER: 0,
            Keyword.FLOAT: 0.0,
            Keyword.VECTOR2: mx.Vector2(),
            Keyword.VECTOR3: mx.Vector3(),
            Keyword.VECTOR4: mx.Vector4(),
            Keyword.COLOR3: mx.Color3(),
            Keyword.COLOR4: mx.Color4()
        }[Keyword(self.__data_type)]

    @property
    def as_token(self) -> Token:
        return Token(self.__data_type)

    def __eq__(self, other: Token | DataType | str) -> bool:
        if isinstance(other, Token):
            return self.__data_type == other.type
        if isinstance(other, DataType):
            return self.__data_type == other.__data_type
        if isinstance(other, str):
            return self.__data_type == other
        return False

    def __hash__(self) -> int:
        return hash(self.__data_type)

    def __str__(self) -> str:
        return self.__data_type


BOOLEAN = DataType(Keyword.BOOLEAN)
INTEGER = DataType(Keyword.INTEGER)
FLOAT = DataType(Keyword.FLOAT)
VECTOR2 = DataType(Keyword.VECTOR2)
VECTOR3 = DataType(Keyword.VECTOR3)
VECTOR4 = DataType(Keyword.VECTOR4)
COLOR3 = DataType(Keyword.COLOR3)
COLOR4 = DataType(Keyword.COLOR4)
STRING = DataType(Keyword.STRING)
FILENAME = DataType(Keyword.FILENAME)
SURFACESHADER = DataType(Keyword.SURFACESHADER)
DISPLACEMENTSHADER = DataType(Keyword.DISPLACEMENTSHADER)
MATERIAL = DataType(Keyword.MATERIAL)

VECTOR_TYPES = {VECTOR2, VECTOR3, VECTOR4}
COLOR_TYPES = {COLOR3, COLOR4}
MULTI_ELEM_TYPES = VECTOR_TYPES | COLOR_TYPES
SHADER_TYPES = {SURFACESHADER, DISPLACEMENTSHADER}
DATA_TYPES = {BOOLEAN, INTEGER, FLOAT, STRING, FILENAME, MATERIAL} | MULTI_ELEM_TYPES | SHADER_TYPES
