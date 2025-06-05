from . import Expression
from .. import mtlx
from ..CompileError import CompileError
from ..Keyword import DataType, INTEGER, FLOAT
from ..Token import Token


# TODO implement nested switch expressions (to get 25 cases)
# TODO implement indexed switch expressions
class SwitchExpression(Expression):
    def __init__(self, token: Token, which: Expression, values: list[Expression]):
        super().__init__(token)
        self.__which = which
        self.__values = values

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.__which.init([INTEGER, FLOAT])
        for value in self.__values:
            value.init(valid_types)

    def _init(self, valid_types: list[DataType]) -> None:
        data_type = self.__values[0].data_type
        for value in self.__values[1:]:
            if value.data_type != data_type:
                raise CompileError("All cases must be the same data type.", self.token)

    @property
    def _data_type(self) -> DataType:
        return self.__values[0].data_type

    def _evaluate(self) -> mtlx.Node:
        node = mtlx.create_node("switch", self.data_type)
        node.set_input("which", self.__which.evaluate())
        for i, value in enumerate(self.__values):
            node.set_input(f"in{i+1}", value.evaluate())
        return node
