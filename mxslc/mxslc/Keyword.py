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
    T = "T"
    BSDF = "BSDF"
    EDF = "EDF"
    VDF = "VDF"

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
            Keyword.T,
            Keyword.BSDF,
            Keyword.EDF,
            Keyword.VDF
        }
