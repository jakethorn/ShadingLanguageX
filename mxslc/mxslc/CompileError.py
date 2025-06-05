from mxslc.Token import Token


class CompileError(Exception):
    def __init__(self, message: str, token: Token = None):
        file = None
        line = None
        if token is not None:
            file = token.file
            line = token.line

        if file is not None and line is not None:
            super().__init__(f"{token.file.name}, line {token.line}: {message}")
        elif line is not None:
            super().__init__(f"line {token.line}: {message}")
        else:
            super().__init__(message)
