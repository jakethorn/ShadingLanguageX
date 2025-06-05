from ..Token import Token
from ..scan import as_token


class Macro:
    def __init__(self, identifier: str | Token, value: str | list[Token] = None):
        self.__identifier = as_token(identifier)
        if value is None:
            self.__value = []
        elif isinstance(value, str):
            self.__value = [as_token(value)]
        else:
            self.__value = value

    @property
    def identifier(self) -> Token:
        return self.__identifier

    @property
    def value(self) -> list[Token]:
        return self.__value


_macros: list[Macro] = []


def define_macro(macro: str | Token | Macro) -> None:
    if isinstance(macro, str | Token):
        macro = Macro(macro)
    if is_macro_defined(macro.identifier):
        # TODO add warning when defining a defined macro
        undefine_macro(macro.identifier)
    _macros.append(macro)


def undefine_macro(identifier: str | Token) -> None:
    identifier = as_token(identifier)
    for macro in _macros:
        if macro.identifier == identifier:
            _macros.remove(macro)
            return
    # TODO add warning when undefining an undefined macro


def is_macro_defined(identifier: str | Token) -> bool:
    identifier = as_token(identifier)
    return identifier in [m.identifier for m in _macros]


def replace_macro(identifier: str | Token) -> list[Token]:
    identifier = as_token(identifier)
    for macro in _macros:
        if macro.identifier == identifier:
            return macro.value
    raise AssertionError()


def undefine_all_macros() -> None:
    _macros.clear()
