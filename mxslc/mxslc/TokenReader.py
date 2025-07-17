from abc import ABC
from collections.abc import Collection

from .CompileError import CompileError
from .Keyword import Keyword
from .Token import Token
from .token_types import IDENTIFIER, STRING_LITERAL, FLOAT_LITERAL


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

    def _consume(self, *token_types: str | Collection[str]) -> Token | None:
        """
        Consume current token if it matches one of the token types.
        """
        token_types = _flatten(token_types)
        token = self._peek()
        if len(token_types) == 0 or token in token_types:
            self.__index += 1
            return token
        return None

    def _match(self, *token_types: str | Collection[str], on_fail: str = None, fail_token: Token = None) -> Token:
        """
        Same as consume, but raise a compile error if no match was found.
        """
        token_types = _flatten(token_types)
        if token := self._consume(token_types):
            return token
        # raise compile error if not a match
        token = self._peek()
        if token in Keyword and token_types == [IDENTIFIER]:
            msg = f"'{token.lexeme}' is a protected keyword and cannot be used as an identifier."
        else:
            msg = f"Expected {_format_tokens(token_types)}, but found '{token.lexeme}'."
        raise CompileError(on_fail or msg, fail_token or token)

    def __peek(self, future: int) -> Token:
        if self.__index + future >= len(self.__tokens):
            raise CompileError(f"Unexpected end of file.", self.__tokens[-1])
        return self.__tokens[self.__index + future]


def _flatten(token_types: tuple[str | Collection[str], ...]) -> list[str]:
    result = []
    for t in token_types:
        if isinstance(t, str):
            result.append(t)
        elif isinstance(t, Collection):
            result.extend(t)
    return result


def _format_tokens(token_types: list[str]) -> str:
    if all(k in token_types for k in Keyword):
        for keyword in Keyword:
            token_types.remove(keyword)
        token_types.insert(0, "a keyword")

    if len(token_types) == 1:
        return _format_token(token_types[0])
    else:
        msg = ", ".join([_format_token(t) for t in token_types[:-1]])
        msg += f" or {_format_token(token_types[-1])}"
        return msg


def _format_token(token_type: str) -> str:
    if token_type == IDENTIFIER:
        return "an identifier"
    if token_type == STRING_LITERAL:
        return "a string literal"
    if token_type == FLOAT_LITERAL:
        return "a float literal"
    return token_type
