from __future__ import annotations

from abc import ABC, abstractmethod

from .. import mx_utils, utils
from ..CompileError import CompileError
from ..DataType import DataType, DATA_TYPES
from ..Token import Token


class Expression(ABC):
    def __init__(self, token: Token | None):
        self.__token = token
        self.__initialized = False

    @property
    def token(self) -> Token:
        return self.__token

    @abstractmethod
    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        ...

    def init(self, valid_types: DataType | set[DataType] = None) -> None:
        if not self.__initialized:
            valid_types = _as_set(valid_types)
            if len(valid_types) == 0:
                raise CompileError("Incompatible data types.", self.token)
            self._init_subexpr(valid_types)
            self._init(valid_types)
            if self._data_type not in valid_types:
                raise CompileError(f"Invalid data type. Expected {utils.types_string(valid_types)}, but got {self._data_type}.", self.token)
            self.__initialized = True

    #virtualmethod
    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        ...

    #virtualmethod
    def _init(self, valid_types: set[DataType]) -> None:
        ...

    @property
    def data_type(self) -> DataType:
        assert self.__initialized
        return self._data_type

    @property
    @abstractmethod
    def _data_type(self) -> DataType:
        ...

    @property
    def data_size(self) -> int:
        return self.data_type.size

    def evaluate(self) -> mx_utils.Node:
        assert self.__initialized
        node = self._evaluate()
        assert node.data_type == self.data_type
        return node

    @abstractmethod
    def _evaluate(self) -> mx_utils.Node:
        ...

    def init_evaluate(self, valid_types: DataType | set[DataType] = None) -> mx_utils.Node:
        self.init(valid_types)
        return self.evaluate()


def _as_set(data_types: DataType | set[DataType]) -> set[DataType]:
    if data_types is None:
        return DATA_TYPES
    if isinstance(data_types, DataType):
        return {data_types}
    elif isinstance(data_types, set):
        return data_types
    raise TypeError
