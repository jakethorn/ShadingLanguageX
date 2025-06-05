import re
from typing import Sequence, Generator, Any

from .Keyword import FLOAT, VECTOR2, VECTOR3, VECTOR4, COLOR3, COLOR4, DataType


def type_of_swizzle(swizzle: str) -> DataType:
    is_vector_swizzle = re.match(r"[xyzw]", swizzle)
    if len(swizzle) == 1:
        return FLOAT
    if len(swizzle) == 2:
        return VECTOR2
    if len(swizzle) == 3:
        return VECTOR3 if is_vector_swizzle else COLOR3
    if len(swizzle) == 4:
        return VECTOR4 if is_vector_swizzle else COLOR4
    raise AssertionError


def as_list(value: Any) -> list[Any] | None:
    if value is None:
        return None
    elif isinstance(value, list):
        return list(set(value))
    else:
        return [value]


def one(values: Sequence[bool] | Generator[bool, None, None]) -> bool:
    return len([v for v in values if v]) == 1


# TODO make this more robust
def is_path(literal: Any) -> bool:
    if not isinstance(literal, str):
        return False
    return literal.endswith(".tif") or literal.endswith(".png") or literal.endswith(".jpg")


def types_string(types: list[DataType]) -> str:
    if len(types) == 1:
        return types[0]
    else:
        return f"<{', '.join(types)}>"


def function_signature_string(name: str, valid_types: list[DataType] | None, args: list["Argument"] | None) -> str:
    output = ""
    if valid_types is not None:
        output += types_string(valid_types) + " "
    output += name
    if args is not None and len(args) > 0:
        output += "("
        for i, arg in enumerate(args):
            index_name = f"arg{i}"
            output += f"{arg.data_type} {index_name if arg.is_positional else arg.name}, "
        output = output[:-2]
        output += ")"
    else:
        output += "()"
    return output
