from __future__ import annotations

from enum import StrEnum, auto


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
    AUTO = auto()
    OUT = auto()
    GLOBAL = auto()
    INLINE = auto()
    CONST = auto()

    # Protected keywords for potential future use
    UNIFORM = auto()
    VARYING = auto()
    NAMESPACE = auto()

    # Data types
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
    MATRIX33 = auto()
    MATRIX44 = auto()
    SURFACESHADER = auto()
    DISPLACEMENTSHADER = auto()
    VOLUMESHADER = auto()
    LIGHTSHADER = auto()
    MATERIAL = auto()
    BSDF = "BSDF"
    EDF = "EDF"
    VDF = "VDF"
    INTEGERARRAY = auto()
    FLOATARRAY = auto()
    STRINGARRAY = auto()
    VECTOR2ARRAY = auto()
    VECTOR3ARRAY = auto()
    VECTOR4ARRAY = auto()
    COLOR3ARRAY = auto()
    COLOR4ARRAY = auto()
    T = "T"

    # Data type aliases
    BOOL = auto()
    INT = auto()
    VEC2 = auto()
    VEC3 = auto()
    VEC4 = auto()

    @staticmethod
    def DATA_TYPES() -> set[Keyword]:
        return {
            Keyword.BOOLEAN,
            Keyword.INTEGER,
            Keyword.FLOAT,
            Keyword.STRING,
            Keyword.FILENAME,
            Keyword.VECTOR2,
            Keyword.VECTOR3,
            Keyword.VECTOR4,
            Keyword.COLOR3,
            Keyword.COLOR4,
            Keyword.MATRIX33,
            Keyword.MATRIX44,
            Keyword.SURFACESHADER,
            Keyword.DISPLACEMENTSHADER,
            Keyword.VOLUMESHADER,
            Keyword.LIGHTSHADER,
            Keyword.MATERIAL,
            Keyword.BSDF,
            Keyword.EDF,
            Keyword.VDF,
            Keyword.INTEGERARRAY,
            Keyword.FLOATARRAY,
            Keyword.STRINGARRAY,
            Keyword.VECTOR2ARRAY,
            Keyword.VECTOR3ARRAY,
            Keyword.VECTOR4ARRAY,
            Keyword.COLOR3ARRAY,
            Keyword.COLOR4ARRAY,
            Keyword.T
        }
