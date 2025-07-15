from . import Expression
from ..CompileError import CompileError
from ..DataType import DataType


def init_linked_expressions(expr1: Expression, expr2: Expression, valid_types: set[DataType]) -> None:
    error1 = expr1.try_init(valid_types)
    error2 = expr2.try_init(valid_types)
    if error1 and error2:
        raise error1
    elif error1:
        expr1.init(expr2.data_type)
    elif error2:
        expr2.init(expr1.data_type)
    if expr1.data_type != expr2.data_type:
        raise CompileError(f"Expressions must evaluate to the same type, but were `{expr1.data_type}` and `{expr2.data_type}`.", expr1.token)


def format_args(args: list["Argument"], *, with_names: bool) -> str:
    result = ""
    if len(args) == 0:
        return result
    for arg in args:
        if with_names and arg.name:
            result += f"{arg.name}={arg.expression}"
        else:
            result += f"{arg.expression}"
        result += ", "
    return result[:-2]
