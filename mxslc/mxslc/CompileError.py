from pathlib import Path


class CompileError(Exception):
    def __init__(self, line: int, message: str, file: Path = None):
        if file is None:
            super().__init__(f"Line {line}: {message}")
        else:
            super().__init__(f"{file.name}, line {line}: {message}")
