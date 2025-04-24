from abc import ABC, abstractmethod


class Statement(ABC):
    @abstractmethod
    def execute(self) -> None:
        ...
