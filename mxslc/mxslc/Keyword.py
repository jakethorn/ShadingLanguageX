from enum import StrEnum, auto
from typing import Any

import MaterialX as mx


class Keyword(StrEnum):
    IF = auto()
    ELSE = auto()
    SWITCH = auto()
    FOR = auto()
    RETURN = auto()
    TRUE = auto()
    FALSE = auto()
    AND = auto()
    OR = auto()
    NOT = auto()
    VOID = auto()
    NULL = auto()


class DataType(StrEnum):
    BOOLEAN = auto()
    INTEGER = auto()
    FLOAT = auto()
    STRING = auto()
    FILENAME = auto()
    VECTOR2 = auto()
    VECTOR3 = auto()
    VECTOR4 = auto()
    COLOR3 = auto()
    COLOR4 = auto()
    SURFACESHADER = auto()
    DISPLACEMENTSHADER = auto()
    MATERIAL = auto()
    T = "T"

    @property
    def size(self) -> int:
        return {BOOLEAN: 1, INTEGER: 1, FLOAT: 1, VECTOR2: 2, VECTOR3: 3, VECTOR4: 4, COLOR3: 3, COLOR4: 4}[self]

    def zeros(self) -> Any:
        return {
            BOOLEAN: False,
            INTEGER: 0,
            FLOAT: 0.0,
            VECTOR2: mx.Vector2(),
            VECTOR3: mx.Vector3(),
            VECTOR4: mx.Vector4(),
            COLOR3: mx.Color3(),
            COLOR4: mx.Color4()
        }[self]


class AliasType(StrEnum):
    BOOL = auto()
    INT = auto()
    VEC2 = auto()
    VEC3 = auto()
    VEC4 = auto()

    @property
    def real(self) -> DataType:
        return {self.VEC2: VECTOR2, self.VEC3: VECTOR3, self.VEC4: VECTOR4, self.BOOL: BOOLEAN, self.INT: INTEGER}[self]


# access data types without qualifier
BOOLEAN = DataType.BOOLEAN
INTEGER = DataType.INTEGER
FLOAT = DataType.FLOAT
STRING = DataType.STRING
FILENAME = DataType.FILENAME
VECTOR2 = DataType.VECTOR2
VECTOR3 = DataType.VECTOR3
VECTOR4 = DataType.VECTOR4
COLOR3 = DataType.COLOR3
COLOR4 = DataType.COLOR4
SURFACESHADER = DataType.SURFACESHADER
DISPLACEMENTSHADER = DataType.DISPLACEMENTSHADER
MATERIAL = DataType.MATERIAL

# common data type groups
VECTOR_TYPES = [DataType.VECTOR2, DataType.VECTOR3, DataType.VECTOR4]
COLOR_TYPES = [DataType.COLOR3, DataType.COLOR4]
FLOAT_TYPES = [DataType.FLOAT, *VECTOR_TYPES, *COLOR_TYPES]
NUMERIC_TYPES = [DataType.INTEGER, *FLOAT_TYPES]
SHADER_TYPES = [DataType.SURFACESHADER, DataType.DISPLACEMENTSHADER]
DATA_TYPES = [t for t in DataType]

# list of all keywords
KEYWORDS = [k for k in Keyword] + DATA_TYPES + [t for t in AliasType]
