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
    SURFACESHADER = auto()
    DISPLACEMENTSHADER = auto()
    MATERIAL = auto()
    T = "T"

    # Data type aliases
    BOOL = auto()
    INT = auto()
    VEC2 = auto()
    VEC3 = auto()
    VEC4 = auto()

    @staticmethod
    def DATA_TYPES() -> list[Keyword]:
        return [
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
            Keyword.SURFACESHADER,
            Keyword.DISPLACEMENTSHADER,
            Keyword.MATERIAL,
            Keyword.T
        ]
