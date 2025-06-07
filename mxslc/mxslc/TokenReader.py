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
        return self.__tokens[self.__index]

    def _peek_next(self) -> Token:
        """
        Peek next token.
        """
        return self.__tokens[self.__index + 1]

    def _peek_next_next(self) -> Token:
        """
        Peek next next token.
        """
        return self.__tokens[self.__index + 2]

    def _consume(self, *token_types: str) -> Token | None:
        """
        Consume current token if it matches one of the token types.
        """
        token = self._peek()
        if len(token_types) == 0 or token in token_types:
            self.__index += 1
            return token
        return None

    # TODO maybe change this (and consume) to take str I list[str] instead of *str
    def _match(self, *token_types: str) -> Token:
        """
        Same as consume, but raise a compile error if no match was found.
        """
        if token := self._consume(*token_types):
            return token
        token = self._peek()
        raise CompileError(f"Expected {token_types}, but found '{token.lexeme}'.", token)
