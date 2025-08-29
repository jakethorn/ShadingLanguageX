from .Expression import Expression
from .FunctionCall import FunctionCall
from ..Argument import Argument
from ..Token import IdentifierToken, Token


# TODO: semantically, it makes more sense for FunctionCall to inherit from NodeExpression
class NodeExpression(FunctionCall):
    def __init__(self, token: Token, name: str, inputs: list[Expression]):
        identifier = IdentifierToken(name, token.file, token.line)
        args = [Argument(a, i) for i, a in enumerate(inputs)]
        super().__init__(identifier, None, args)
