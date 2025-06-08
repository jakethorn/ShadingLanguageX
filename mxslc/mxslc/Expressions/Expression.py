from __future__ import annotations

from abc import ABC, abstractmethod

from .. import mx_utils
from ..CompileError import CompileError
from ..DataType import DataType, INTEGER, FLOAT
from ..Keyword import Keyword
from ..Token import Token
from ..utils import as_list


class Expression(ABC):
    def __init__(self, token: Token | None):
        self.__token = token
        self.__initialized = False
        self.__valid_types = []

    @abstractmethod
    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        ...

    def init(self, valid_types: DataType | list[DataType] = None) -> None:
        if not self.__initialized:
            self.__valid_types = as_list(valid_types) or Keyword.DATA_TYPES()
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

    def evaluate(self) -> mx_utils.Node:
        assert self.__initialized
        node = self._evaluate()
        assert node.data_type == self.data_type
        node = _implicit_int_to_float(node, self.__valid_types)
        if node.data_type not in self.__valid_types:
            raise CompileError(f"Invalid data type. Expected one of {self.__valid_types}, but got {node.data_type}.", self.token)

        return node

    @abstractmethod
    def _evaluate(self) -> mx_utils.Node:
        ...

    def init_evaluate(self, valid_types: DataType | list[DataType] = None) -> mx_utils.Node:
        self.init(valid_types)
        return self.evaluate()


def _implicit_int_to_float(node: mx_utils.Node, valid_types: list[DataType]) -> mx_utils.Node:
    is_int = node.data_type == INTEGER
    int_is_valid = INTEGER in valid_types
    float_is_valid = FLOAT in valid_types
    if is_int and not int_is_valid and float_is_valid:
        return mx_utils.convert(node, FLOAT)
    else:
        return node
