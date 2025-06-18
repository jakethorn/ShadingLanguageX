
class Argument:
    def __init__(self, name: str, value: str):
        self.__name = name
        self.__value = value

    @property
    def name(self) -> str:
        return self.__name

    @property
    def value(self) -> str:
        return self.__value

    def __str__(self):
        return f"{self.name}={self.value}"
