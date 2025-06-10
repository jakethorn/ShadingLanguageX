from abc import ABC

from .CompileError import CompileError
from .Token import Token


class TokenReader(ABC):
    def __init__(self, tokens: list[Token]):
        self.__tokens = tokens
        self.__index = 0

    def _reading_tokens(self) -> bool:
        """
        Returns true if there are more tokens to read.
        """
        return self.__index < len(self.__tokens)

    def _peek(self) -> Token:
        """
        Peek current token.
        """
        return self.__peek(0)

    def _peek_next(self) -> Token:
        """
        Peek next token.
        """
        return self.__peek(1)

    def _peek_next_next(self) -> Token:
        """
        Peek next next token.
        """
        return self.__peek(2)

    def _consume(self, *token_types: str | list[str]) -> Token | None:
        """
        Consume current token if it matches one of the token types.
        """
        token_types = _flatten(token_types)
        token = self._peek()
        if len(token_types) == 0 or token in token_types:
            self.__index += 1
            return token
        return None

    def _match(self, *token_types: str | list[str]) -> Token:
        """
        Same as consume, but raise a compile error if no match was found.
        """
        token_types = _flatten(token_types)
        if token := self._consume(token_types):
            return token
        token = self._peek()
        raise CompileError(f"Expected {token_types}, but found '{token.lexeme}'.", token)

    def __peek(self, future: int) -> Token:
        if self.__index + future >= len(self.__tokens):
            raise CompileError(f"Unexpected end of file.", self.__tokens[-1])
        return self.__tokens[self.__index + future]


def _flatten(token_types: tuple[str | list[str], ...]) -> list[str]:
    result = []
    for t in token_types:
        if isinstance(t, str):
            result.append(t)
        elif isinstance(t, list):
            result.extend(t)
    return result
