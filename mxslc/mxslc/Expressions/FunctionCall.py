from typing import Any

from . import Expression
from .expression_utils import format_args
from .. import state, utils
from ..CompileError import CompileError
from ..DataType import DataType
from ..Token import Token
from ..mx_wrapper import Node

type Argument = Any


class FunctionCall(Expression):
    """
    Represents a call to a user-defined or standard library function.
    """
    def __init__(self, identifier: Token, template_type: Token | DataType | None, args: list[Argument]):
        super().__init__(identifier)
        self.__identifier = identifier
        self.__template_type = DataType(template_type)
        self.__args = args
        self.__func = None

        self.__assert_valid_argument_order()

    @property
    def __initialized_args(self) -> list[Argument]:
        return [a for a in self.__args if a.is_initialized]

    @property
    def __uninitialized_args(self) -> list[Argument]:
        return [a for a in self.__args if not a.is_initialized]

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        if self.__template_type:
            data_type = self.__template_type.instantiate(template_type)
        else:
            data_type = None
        args = [a.instantiate_templated_types(template_type) for a in self.__args]
        return FunctionCall(self.__identifier, data_type, args)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        while len(self.__uninitialized_args) > 0:
            is_progress = False
            errors: dict[Argument, CompileError] = {}
            for arg in self.__uninitialized_args:
                param_index = arg.position if arg.is_positional else arg.name
                valid_arg_types = state.get_function_parameter_types(valid_types, self.__identifier, self.__template_type, self.__initialized_args, param_index)
                if len(valid_arg_types) == 0:
                    raise CompileError(f"Function signature '{utils.format_function(valid_types, self.__identifier.lexeme, self.__template_type, None)}' does not exist.", self.__identifier)
                errors[arg] = arg.try_init(valid_arg_types)
                is_progress |= arg.is_initialized
            if not is_progress:
                self.__raise_subexpr_error(valid_types, errors)

    def __raise_subexpr_error(self, valid_types: set[DataType], errors: dict[Argument, CompileError]) -> str:
        msg = f"Invalid call to '{self.__identifier}':\n"
        for arg, error in errors.items():
            if error is not None:
                msg += f"Argument {arg.position+1}: {error}\n"
        msg += "Possible function signatures:\n"
        msg += "\n".join([str(f) for f in state.get_functions(self.__identifier.lexeme, self.__template_type, valid_types, self.__initialized_args, strict_args=False)])
        raise CompileError(msg)

    def _init(self, valid_types: set[DataType]) -> None:
        self.__func = state.get_function(self.__identifier, self.__template_type, valid_types, self.__args)

    @property
    def _data_type(self) -> DataType:
        return self.__func.return_type

    def _evaluate(self) -> Node:
        return self.__func.invoke(self.__args)

    def _set_argument(self, arg: Argument) -> None:
        self.__args[arg.position] = arg

    def __assert_valid_argument_order(self):
        found_named = False
        for arg in self.__args:
            if arg.is_named:
                found_named = True
            if found_named and arg.is_positional:
                raise CompileError("Named arguments must come after positional arguments.", self.__identifier)

    def __str__(self) -> str:
        if self.__template_type:
            return f"{self.__identifier}<{self.__template_type}>({format_args(self.__args, with_names=True)})"
        else:
            return f"{self.__identifier}({format_args(self.__args, with_names=True)})"

