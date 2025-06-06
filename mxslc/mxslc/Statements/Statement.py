from __future__ import annotations
from abc import ABC, abstractmethod

from mxslc.Keyword import DataType


class Statement(ABC):
    @abstractmethod
    def instantiate_templated_types(self, data_type: DataType) -> Statement:
        ...

    @abstractmethod
    def execute(self) -> None:
        ...
