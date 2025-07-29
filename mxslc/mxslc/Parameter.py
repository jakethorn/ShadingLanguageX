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
    def __init__(self, identifier: Token, data_type: Token | DataType, default_value: Expression = None, is_out=False):
        self.__identifier = identifier
        self.__data_type = DataType(data_type)
        self.__default_value = default_value
        self.__is_out = is_out

    @property
    def name(self) -> str:
        return self.__identifier.lexeme

    @property
    def identifier(self) -> Token:
        return self.__identifier

    @property
    def data_type(self) -> DataType:
        return self.__data_type

    @property
    def default_value(self) -> Expression | None:
        return self.__default_value

    @property
    def is_in(self) -> bool:
        return not self.__is_out

    @property
    def is_out(self) -> bool:
        return self.__is_out

    def init_default_value(self) -> None:
        if self.default_value:
            self.default_value.init(self.data_type)

    def __str__(self) -> str:
        output = ""
        if self.is_out:
            output += "out "
        output += f"{self.data_type} {self.name}"
        if self.default_value:
            output += f" = {self.default_value}"
        return output


class ParameterList:
    """
    A list of parameters that can be accessed by their position or name.
    """
    def __init__(self, params: ParameterList | list[Parameter] = None):
        if isinstance(params, ParameterList):
            self.__params: list[Parameter] = params.__params
        elif isinstance(params, list):
            self.__params: list[Parameter] = params
        else:
            self.__params: list[Parameter] = []

    @property
    def ins(self) -> ParameterList:
        return ParameterList([p for p in self if p.is_in])

    @property
    def outs(self) -> ParameterList:
        return ParameterList([p for p in self if p.is_out])

    def instantiate_templated_parameters(self, template_type: DataType) -> ParameterList:
        return ParameterList([
            Parameter(p.identifier, p.data_type.instantiate(template_type), p.default_value)
            for p
            in self.__params
        ])

    def init_default_values(self) -> None:
        for param in self.__params:
            param.init_default_value()

    def __iadd__(self, param: Parameter) -> ParameterList:
        self.__params.append(param)
        return self

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

    def __str__(self) -> str:
        return ", ".join([str(p) for p in self.__params])
