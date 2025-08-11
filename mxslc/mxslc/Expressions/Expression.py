
from __future__ import annotations
from typing import Union
from abc import ABC, abstractmethod

from .. import utils
from ..CompileError import CompileError
from ..DataType import DataType, DATA_TYPES, VOID
from ..Keyword import Keyword
from ..Token import Token
from ..mx_wrapper import Node


class Expression(ABC):
    def __init__(self, token: Union[Token, None]):
        self.__token = token
        self.__initialized = False

    @property
    def token(self) -> Token:
        return self.__token

    @property
    def is_initialized(self) -> bool:
        return self.__initialized

    @abstractmethod
    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        ...

    def init(self, valid_types: Union[DataType, set[DataType]] = None) -> None:
        if not self.__initialized:
            valid_types = _handle_valid_types(valid_types)
            if len(valid_types) == 0:
                raise CompileError("Incompatible data types.", self.token)
            self._init_subexpr(valid_types)
            self._init(valid_types)
            if self._data_type not in valid_types:
                raise CompileError(f"Invalid data type. Expected {utils.format_types(valid_types)}, but got {self._data_type}.", self.token)
            self.__initialized = True

    def try_init(self, valid_types: Union[DataType, set[DataType]] = None) -> Union[CompileError, None]:
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
        return self._data_type

    @property
    @abstractmethod
    def _data_type(self) -> DataType:
        ...

    @property
    def data_size(self) -> int:
        return self.data_type.size

    def evaluate(self) -> Node:
        assert self.__initialized
        node = self._evaluate()
        assert (self.data_type == VOID) or (node.data_type == self.data_type)
        return node

    @abstractmethod
    def _evaluate(self) -> Node:
        ...

    def init_evaluate(self, valid_types: Union[DataType, set[DataType]] = None) -> Node:
        self.init(valid_types)
        return self.evaluate()


def _handle_valid_types(data_types: Union[DataType, set[DataType]]) -> set[DataType]:
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
