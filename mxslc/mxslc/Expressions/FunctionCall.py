from . import Expression
from .. import state, mtlx
from ..Keyword import DataType
from ..Token import Token


class FunctionCall(Expression):
    """
    Represents a call to a user-defined function.
    """
    def __init__(self, identifier: Token, args: list["Argument"]):
        super().__init__(identifier)
        self.__identifier = identifier
        self.__args = args
        self.__func = None

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        args = [a.instantiate_templated_types(data_type) for a in self.__args]
        return FunctionCall(self.__identifier, args)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        for i, arg in enumerate(self.__args):
            param_index = i if arg.is_positional else arg.name
            valid_arg_types = state.get_function_parameter_types(self.__identifier, param_index)
            arg.expression.init(valid_arg_types)

    def _init(self, valid_types: list[DataType]) -> None:
        self.__func = state.get_function(self.__identifier, valid_types, self.__args)

    @property
    def _data_type(self) -> DataType:
        return self.__func.return_type

    def _evaluate(self) -> mtlx.Node:
        return self.__func.invoke(self.__args)
