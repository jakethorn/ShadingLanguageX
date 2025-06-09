import re
from typing import Sequence, Generator, Any

from mxslc.DataType import DataType, FLOAT, VECTOR2, VECTOR3, VECTOR4, COLOR4, COLOR3, DATA_TYPES


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


def one(values: Sequence[bool] | Generator[bool, None, None]) -> bool:
    return len([v for v in values if v]) == 1


# TODO make this more robust
def is_path(literal: Any) -> bool:
    if not isinstance(literal, str):
        return False
    return literal.endswith(".tif") or literal.endswith(".png") or literal.endswith(".jpg")


def string(value: Any) -> str | None:
    if value is None:
        return None
    else:
        return str(value)


def types_string(types: set[DataType]) -> str:
    if len(types) == 1:
        return str(list(types)[0])
    elif types == DATA_TYPES:
        return "any"
    else:
        return f"<{', '.join([str(t) for t in types])}>"


def function_signature_string(return_types: set[DataType] | None, name: str, template_type: DataType | None, args: list["Argument"] | None) -> str:
    output = ""
    if return_types:
        output += types_string(return_types) + " "
    output += name
    if template_type:
        output += f"<{template_type}>"
    if args:
        output += "("
        for arg in args:
            if arg.is_positional:
                output += f"{arg.data_type} arg{arg.position}, "
        if any(arg.is_named for arg in args):
            output += "..., "
            for arg in args:
                if arg.is_named:
                    output += f"{arg.data_type} {arg.name}, "
            output += "...)"
        else:
            output = output[:-2]
            output += ")"
    else:
        output += "()"
    return output
