from __future__ import annotations

from abc import ABC, abstractmethod
from pathlib import Path

from .. import utils, node_utils
from ..CompileError import CompileError
from ..DataType import DataType, DATA_TYPES, VOID, STRING, FILENAME, FLOAT, INTEGER
from ..Keyword import Keyword
from ..Token import Token
from ..mx_wrapper import Node, Uniform


class Expression(ABC):
    def __init__(self, token: Token | None):
        self.__token = token
        self.__initialized = False
        self.__converted_type: DataType | None = None

    @property
    def token(self) -> Token | None:
        return self.__token

    @property
    def is_initialized(self) -> bool:
        return self.__initialized

    @abstractmethod
    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        ...

    def init(self, valid_types: DataType | set[DataType] = None) -> None:
        if not self.__initialized:
            valid_types = _handle_valid_types(valid_types)
            if len(valid_types) == 0:
                raise CompileError("Incompatible data types.", self.token)
            self._init_subexpr(valid_types)
            self._init(valid_types)
            self.__initialized = True
            if self._data_type not in valid_types and not self.__try_convert(valid_types):
                raise CompileError(f"Invalid data type. Expected {utils.format_types(valid_types)}, but got {self._data_type}.", self.token)

    def __try_convert(self, valid_types: set[DataType]) -> bool:
        implicit_conversions = {
            STRING: FILENAME,
            INTEGER: FLOAT
        }
        if self.has_value and self._data_type in implicit_conversions and implicit_conversions[self._data_type] in valid_types:
            self.__converted_type = implicit_conversions[self._data_type]
            return True
        return False

    def try_init(self, valid_types: DataType | set[DataType] = None) -> CompileError | None:
        try:
            self.init(valid_types)
            return None
        except CompileError as e:
            return e

    #virtualmethod
    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        ...

    #virtualmethod
    def _init(self, valid_types: set[DataType]) -> None:
        ...

    @property
    def data_type(self) -> DataType:
        assert self.__initialized
        return self.__converted_type or self._data_type

    @property
    @abstractmethod
    def _data_type(self) -> DataType:
        ...

    @property
    def data_size(self) -> int:
        return self.data_type.size

    @property
    def value(self) -> Uniform | None:
        assert self.__initialized
        if self.__converted_type is None:
            return self._value
        else:
            implicit_conversions = {
                FILENAME: lambda s: Path(s),
                FLOAT: lambda i: float(i)
            }
            conversion = implicit_conversions[self.__converted_type]
            converted_value = conversion(self._value)
            return converted_value

    @property
    #virtualmethod
    def _value(self) -> Uniform | None:
        return None

    @property
    def has_value(self) -> bool:
        return self._value is not None

    def evaluate(self) -> Node:
        assert self.__initialized
        if self.has_value:
            node = node_utils.constant(self.value)
        else:
            node = self._evaluate()
        assert (self.data_type == VOID) or (node.data_type == self.data_type)
        return node

    @abstractmethod
    def _evaluate(self) -> Node:
        ...

    def init_evaluate(self, valid_types: DataType | set[DataType] = None) -> Node:
        self.init(valid_types)
        return self.evaluate()


def _handle_valid_types(data_types: DataType | set[DataType]) -> set[DataType]:
    if data_types is None:
        return DATA_TYPES ^ {VOID}
    if isinstance(data_types, DataType):
        if data_types == Keyword.AUTO:
            return DATA_TYPES
        else:
            return {data_types}
    elif isinstance(data_types, set):
        return data_types
    raise TypeError
