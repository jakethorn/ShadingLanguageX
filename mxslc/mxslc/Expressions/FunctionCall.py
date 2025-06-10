from . import Expression
from .. import state, mx_utils, utils
from ..CompileError import CompileError
from ..DataType import DataType
from ..Token import Token


class FunctionCall(Expression):
    """
    Represents a call to a user-defined or standard library function.
    """
    def __init__(self, identifier: Token, template_type: Token | DataType | None, args: list["Argument"]):
        super().__init__(identifier)
        self.__identifier = identifier
        self.__template_type = DataType(template_type)
        self.__args = args
        self.__func = None

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        if self.__template_type:
            data_type = self.__template_type.instantiate(template_type)
        else:
            data_type = None
        args = [a.instantiate_templated_types(template_type) for a in self.__args]
        return FunctionCall(self.__identifier, data_type, args)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        # TODO this can be improved by passing the previously initialised arg to the following arg and filtering possibilities based on the previous arg type.
        for arg in self.__args:
            param_index = arg.position if arg.is_positional else arg.name
            valid_arg_types = state.get_function_parameter_types(valid_types, self.__identifier, self.__template_type, param_index)
            if len(valid_arg_types) == 0:
                raise CompileError(f"Function signature '{utils.function_signature_string(valid_types, self.__identifier.lexeme, self.__template_type, None)}' does not exist.", self.__identifier)
            arg.init(valid_arg_types)

    def _init(self, valid_types: set[DataType]) -> None:
        self.__func = state.get_function(self.__identifier, self.__template_type, valid_types, self.__args)

    @property
    def _data_type(self) -> DataType:
        return self.__func.return_type

    def _evaluate(self) -> mx_utils.Node:
        return self.__func.invoke(self.__args)
