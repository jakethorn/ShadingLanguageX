from __future__ import annotations
from abc import ABC, abstractmethod

from ..DataType import DataType


class Statement(ABC):
    @abstractmethod
    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        ...

    @abstractmethod
    def execute(self) -> None:
        ...
