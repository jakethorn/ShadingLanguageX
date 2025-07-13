from . import Expression
from .. import node_utils
from ..CompileError import CompileError
from ..DataType import DataType, INTEGER, FLOAT
from ..mx_wrapper import Node


# TODO implement nested switch expressions (to get 25 cases)
# TODO implement indexed switch expressions
class SwitchExpression(Expression):
    def __init__(self, which: Expression, values: list[Expression]):
        super().__init__(which.token)
        self.__which = which
        self.__values = values

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        which = self.__which.instantiate_templated_types(template_type)
        values = [v.instantiate_templated_types(template_type) for v in self.__values]
        return SwitchExpression(which, values)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.__which.init({INTEGER, FLOAT})
        for value in self.__values:
            value.init(valid_types)

    def _init(self, valid_types: set[DataType]) -> None:
        data_type = self.__values[0].data_type
        for value in self.__values[1:]:
            if value.data_type != data_type:
                raise CompileError("All switch cases must evaluate to the same data type.", self.token)

    @property
    def _data_type(self) -> DataType:
        return self.__values[0].data_type

    def _evaluate(self) -> Node:
        node = node_utils.create("switch", self.data_type)
        node.set_input("which", self.__which.evaluate())
        for i, value in enumerate(self.__values):
            node.set_input(f"in{i + 1}", value.evaluate())
        return node
