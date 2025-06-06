from __future__ import annotations
from typing import Iterator

from .Expressions import Expression
from .Keyword import DataType
from .Token import Token


class Parameter:
    """
    Represents a parameter to a function or constructor call.
    """
    def __init__(self, name: Token | str, data_type: Token | DataType, default_value: Expression = None):
        if isinstance(name, Token):
            self.__name_token = name
            self.__name = name.lexeme
        else:
            self.__name_token = None
            self.__name = name

        if isinstance(data_type, Token):
            self.__data_type_token = data_type
            self.__data_type = DataType(data_type.type)
        else:
            self.__data_type_token = None
            self.__data_type = data_type

        self.__default_value = default_value

    @property
    def name(self) -> str:
        return self.__name

    @property
    def data_type(self) -> DataType:
        return self.__data_type

    @property
    def name_token(self) -> Token:
        return self.__name_token

    @property
    def data_type_token(self) -> Token:
        return self.__data_type_token

    @property
    def default_value(self) -> Expression:
        return self.__default_value


class ParameterList:
    """
    A list of parameters that can be accessed by their position or name.
    """
    def __init__(self, params: list[Parameter]):
        self.__params = params

    @property
    def names(self) -> list[str]:
        return [p.name for p in self.__params]

    def instantiate_templated_parameters(self, data_type: DataType) -> ParameterList:
        params = []
        for param in self.__params:
            if param.data_type is DataType.T:
                params.append(Parameter(param.name_token, data_type, param.default_value))
            else:
                params.append(param)
        return ParameterList(params)

    def __getitem__(self, index: int | str) -> Parameter:
        if isinstance(index, int):
            return self.__params[index]
        else:
            for param in self.__params:
                if param.name == index:
                    return param
        raise IndexError(f"No parameter found with the name '{index}'.")

    def __len__(self) -> int:
        return len(self.__params)

    def __contains__(self, param_name: str) -> bool:
        for param in self.__params:
            if param.name == param_name:
                return True
        return False

    def __iter__(self) -> Iterator[Parameter]:
        yield from self.__params
