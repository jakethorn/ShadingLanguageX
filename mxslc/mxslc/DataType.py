from __future__ import annotations

from pathlib import Path
from typing import Any

import MaterialX as mx

from .Keyword import Keyword
from .Token import Token

type Uniform = Any


class DataType:
    """
    Represents a data type (e.g., float, vector3, string, etc...).
    """
    def __new__(cls, data_type: Token | DataType | str):
        if data_type is None:
            return None
        else:
            return super().__new__(cls)

    def __init__(self, data_type: Token | DataType | str):
        if isinstance(data_type, Token):
            self.__data_type = data_type.type
        elif isinstance(data_type, DataType):
            self.__data_type = data_type.__data_type
        elif isinstance(data_type, str):
            self.__data_type = data_type
        else:
            raise TypeError
        assert self.__data_type in Keyword.DATA_TYPES() ^ {Keyword.VOID, Keyword.AUTO}, self.__data_type

    def instantiate(self, template_type: DataType | None) -> DataType:
        if self.__data_type == Keyword.T and template_type:
            return DataType(template_type)
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

    @property
    def as_token(self) -> Token:
        return Token(self.__data_type)

    def from_value(self, value: bool | int | float) -> Uniform:
        value = float(value)
        if self.__data_type == Keyword.BOOLEAN: return bool(value)
        if self.__data_type == Keyword.INTEGER: return int(value)
        if self.__data_type == Keyword.FLOAT:   return value
        if self.__data_type == Keyword.VECTOR2: return mx.Vector2(value)
        if self.__data_type == Keyword.VECTOR3: return mx.Vector3(value)
        if self.__data_type == Keyword.VECTOR4: return mx.Vector4(value)
        if self.__data_type == Keyword.COLOR3:  return mx.Color3(value)
        if self.__data_type == Keyword.COLOR4:  return mx.Color4(value)
        raise AssertionError

    def from_channels(self, channels: list[float]) -> mx.VectorBase:
        channels = channels[:self.size]
        if self.__data_type == Keyword.VECTOR2: return mx.Vector2(channels)
        if self.__data_type == Keyword.VECTOR3: return mx.Vector3(channels)
        if self.__data_type == Keyword.VECTOR4: return mx.Vector4(channels)
        if self.__data_type == Keyword.COLOR3:  return mx.Color3(channels)
        if self.__data_type == Keyword.COLOR4:  return mx.Color4(channels)
        raise AssertionError

    def zeros(self) -> Uniform:
        return self.from_value(0)

    def default(self) -> Uniform:
        if self.__data_type == Keyword.BOOLEAN:  return False
        if self.__data_type == Keyword.INTEGER:  return 0
        if self.__data_type == Keyword.FLOAT:    return 0.0
        if self.__data_type == Keyword.VECTOR2:  return mx.Vector2()
        if self.__data_type == Keyword.VECTOR3:  return mx.Vector3()
        if self.__data_type == Keyword.VECTOR4:  return mx.Vector4()
        if self.__data_type == Keyword.COLOR3:   return mx.Color3()
        if self.__data_type == Keyword.COLOR4:   return mx.Color4()
        if self.__data_type == Keyword.STRING:   return ""
        if self.__data_type == Keyword.FILENAME: return Path()
        return ""

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
VOLUMESHADER = DataType(Keyword.VOLUMESHADER)
LIGHTSHADER = DataType(Keyword.LIGHTSHADER)
MATERIAL = DataType(Keyword.MATERIAL)
BSDF = DataType(Keyword.BSDF)
EDF = DataType(Keyword.EDF)
VDF = DataType(Keyword.VDF)
VOID = DataType(Keyword.VOID)

VECTOR_TYPES = {VECTOR2, VECTOR3, VECTOR4}
COLOR_TYPES = {COLOR3, COLOR4}
MULTI_ELEM_TYPES = VECTOR_TYPES | COLOR_TYPES
SHADER_TYPES = {SURFACESHADER, DISPLACEMENTSHADER, VOLUMESHADER, LIGHTSHADER}
DIST_FUNC_TYPES = {BSDF, EDF, VDF}
DATA_TYPES = {BOOLEAN, INTEGER, FLOAT, STRING, FILENAME, MATERIAL} | MULTI_ELEM_TYPES | SHADER_TYPES | DIST_FUNC_TYPES
