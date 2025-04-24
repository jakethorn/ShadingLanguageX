from ..Token import Token


__macros: list[tuple[Token, list[Token]]] = []


def define(identifier: str | Token, value: list[Token] = None) -> None:
    identifier = __as_token(identifier)

    if is_defined(identifier):
        # TODO add warning when defining a defined macro
        undefine(identifier)

    __macros.append((identifier, value or []))


def undefine(identifier: str | Token) -> None:
    identifier = __as_token(identifier)
    for macro in __macros:
        if macro[0] == identifier:
            __macros.remove(macro)
            return
    # TODO add warning when undefining an undefined macro


def is_defined(identifier: str | Token) -> bool:
    identifier = __as_token(identifier)
    return identifier in [m[0] for m in __macros]


def replace(identifier: str | Token) -> list[Token]:
    identifier = __as_token(identifier)
    for macro in __macros:
        if macro[0] == identifier:
            return macro[1]
    raise AssertionError()


def clear() -> None:
    __macros.clear()


def __as_token(identifier: str | Token) -> Token:
    if isinstance(identifier, Token):
        return identifier
    return Token(identifier)
