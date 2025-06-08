from __future__ import annotations
from typing import Iterator

from .Argument import Argument
from .DataType import DataType
from .Expressions import Expression
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
        return ParameterList([
            Parameter(p.name_token, p.data_type.instantiate(data_type), p.default_value)
            for p
            in self.__params
        ])

    def __getitem__(self, index: int | str | Argument) -> Parameter:
        if isinstance(index, int) and index < len(self.__params):
            return self.__params[index]
        elif isinstance(index, str):
            for param in self.__params:
                if param.name == index:
                    return param
        elif isinstance(index, Argument):
            if index.is_positional:
                param = self[index.position]
            else:
                param = self[index.name]
            if param.data_type == index.data_type:
                return param
        raise IndexError(f"No parameter found with the index '{index}'.")

    def __len__(self) -> int:
        return len(self.__params)

    def __contains__(self, index: int | str | Parameter) -> bool:
        if isinstance(index, int):
            return index < len(self.__params)
        if isinstance(index, str):
            for param in self.__params:
                if param.name == index:
                    return True
            return False
        if isinstance(index, Parameter):
            return index in self.__params
        raise TypeError

    def __iter__(self) -> Iterator[Parameter]:
        yield from self.__params
