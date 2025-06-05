from __future__ import annotations

from abc import ABC, abstractmethod

from .. import mtlx
from ..CompileError import CompileError
from ..Keyword import DataType, INTEGER, FLOAT, DATA_TYPES
from ..Token import Token
from ..utils import as_list


class Expression(ABC):
    def __init__(self, token: Token | None):
        self.__token = token
        self.__initialized = False
        self.__valid_types = []

    def init(self, valid_types: DataType | list[DataType] = None) -> None:
        if not self.__initialized:
            self.__valid_types = as_list(valid_types) or DATA_TYPES
            self._init_subexpr(self.__valid_types)
            self._init(self.__valid_types)
            self.__initialized = True

    #virtualmethod
    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        ...

    #virtualmethod
    def _init(self, valid_types: list[DataType]) -> None:
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

    @property
    def token(self) -> Token:
        return self.__token

    def evaluate(self) -> mtlx.Node:
        assert self.__initialized
        node = self._evaluate()
        assert node.data_type == self.data_type
        node = _implicit_int_to_float(node, self.__valid_types)
        if node.data_type not in self.__valid_types:
            raise CompileError(f"Invalid data type. Expected one of {self.__valid_types}, but got {node.data_type}.", self.token)

        return node

    @abstractmethod
    def _evaluate(self) -> mtlx.Node:
        ...

    def init_evaluate(self, valid_types: DataType | list[DataType] = None) -> mtlx.Node:
        self.init(valid_types)
        return self.evaluate()


def _implicit_int_to_float(node: mtlx.Node, valid_types: list[DataType]) -> mtlx.Node:
    is_int = node.data_type is INTEGER
    int_is_valid = INTEGER in valid_types
    float_is_valid = FLOAT in valid_types
    if is_int and not int_is_valid and float_is_valid:
        return mtlx.convert(node, FLOAT)
    else:
        return node
