from ..mx_wrapper import Element


# TODO add line numbers to decompilation errors
class DecompileError(Exception):
    def __init__(self, message: str, element: Element):
        super().__init__(f"{message}: {element}")
