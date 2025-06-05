from .Expressions import Expression
from .Keyword import DataType
from .Token import Token

type ParameterTuple = tuple[str, DataType | list[DataType]]


class Parameter:
    """
    Represents a parameter to a function, constructor or standard library call.
    Function and constructor calls will only ever have one data_type, but standard library calls can have multple.
    """
    def __init__(self, name: Token | str, data_types: Token | DataType | list[DataType], default_value: Expression = None):
        if isinstance(name, Token):
            self.__name_token = name
            self.__name = name.lexeme
        else:
            self.__name_token = None
            self.__name = name

        if isinstance(data_types, Token):
            self.__data_type_token = data_types
            self.__data_types = [DataType(data_types.type)]
        else:
            self.__data_type_token = None
            self.__data_types = data_types if isinstance(data_types, list) else [data_types]

        self.__default_value = default_value

    @property
    def name(self) -> str:
        return self.__name

    @property
    def data_type(self) -> DataType:
        return self.__data_types[0]

    @property
    def data_types(self) -> list[DataType]:
        return self.__data_types

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
    def __init__(self, parameters: list[Parameter | ParameterTuple]):
        self.__parameters = [p if isinstance(p, Parameter) else Parameter(*p) for p in parameters]

    def __getitem__(self, index: int | str) -> Parameter:
        if isinstance(index, int):
            return self.__parameters[index]
        else:
            for param in self.__parameters:
                if param.name == index:
                    return param
        raise IndexError(f"No parameter found with the name '{index}'.")

    def __len__(self) -> int:
        return len(self.__parameters)

    def __contains__(self, param_name: str) -> bool:
        for param in self.__parameters:
            if param.name == param_name:
                return True
        return False
