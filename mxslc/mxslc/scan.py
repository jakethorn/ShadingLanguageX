import re
from pathlib import Path

from .Keyword import KEYWORDS
from .Token import Token
from .token_types import EOF, IDENTIFIER, FLOAT_LITERAL, INT_LITERAL, FILENAME_LITERAL, STRING_LITERAL, EOL, COMMENT


def scan(file: Path) -> list[Token]:
    return Scanner(file).scan()


class Scanner:
    def __init__(self, file: Path):
        self.__file = file
        with open(file, "r") as f:
            self.__source = f.read()
        self.__index = 0
        self.__line = 1

    def scan(self) -> list[Token]:
        tokens = []
        while self.__index < len(self.__source):
            token = self.__identify_token()
            if token is not None:
                if token.type != COMMENT:
                    tokens.append(token)
                if token.type == EOL:
                    self.__line += 1
                self.__index += len(token.lexeme)
            else:
                self.__index += 1
        tokens.append(self.__token(EOF))
        return tokens

    def __identify_token(self) -> Token | None:
        if comment := self.__get_line_comment():
            return self.__token(COMMENT, comment)
        char = self.__peek()
        if char in ["(", ")", "{", "}", "[", "]", ".", ",", ":", ";", EOL]:
            return self.__token(char)
        if char in ["!", "=", ">", "<", "+", "-", "*", "/", "%", "^", "&", "|"]:
            if self.__peek_next() == "=":
                return self.__token(char + "=")
            else:
                return self.__token(char)
        if directive := self.__get_directive():
            return self.__token(directive)
        if word := self.__get_word():
            if word in KEYWORDS:
                return self.__token(word)
            else:
                return self.__token(IDENTIFIER, word)
        if float_lit := self.__get_float_literal():
            return self.__token(FLOAT_LITERAL, float_lit)
        if int_lit := self.__get_int_literal():
            return self.__token(INT_LITERAL, int_lit)
        if filename_lit := self.__get_filename_literal():
            return self.__token(FILENAME_LITERAL, filename_lit)
        if string_lit := self.__get_string_literal():
            return self.__token(STRING_LITERAL, string_lit)
        return None

    def __peek(self) -> str:
        return self.__source[self.__index]

    def __peek_next(self) -> str | None:
        i = self.__index + 1
        return self.__source[i] if i < len(self.__source) else None

    def __peek_all(self) -> str:
        return self.__source[self.__index:]

    def __get_line_comment(self) -> str | None:
        match = re.match(r"//.*", self.__peek_all())
        return match.group() if match else None

    def __get_directive(self) -> str | None:
        match = re.match(r"#[a-z]*", self.__peek_all())
        return match.group() if match else None

    def __get_word(self) -> str | None:
        match = re.match(r"[_a-zA-Z][_a-zA-Z0-9]*", self.__peek_all())
        return match.group() if match else None

    def __get_float_literal(self) -> str | None:
        match = re.match(r"[0-9]+\.[0-9]+", self.__peek_all())
        return match.group() if match else None

    def __get_int_literal(self) -> str | None:
        match = re.match(r"[0-9]+", self.__peek_all())
        return match.group() if match else None

    def __get_filename_literal(self) -> str | None:
        match = re.match(r'"[^"]*\.(mxsl|mtlx|tif|png|jpg)"', self.__peek_all())
        return match.group() if match else None

    def __get_string_literal(self) -> str | None:
        match = re.match(r'"[^"]*"', self.__peek_all())
        return match.group() if match else None

    def __token(self, type_: str, lexeme: str = None) -> Token:
        return Token(type_, lexeme, self.__file, self.__line)
