from . import Expression
from .expression_utils import format_args
from .. import mx_utils
from ..CompileError import CompileError
from ..DataType import DataType
from ..Token import Token


class NodeConstructor(Expression):
    def __init__(self, category: Token, data_type: Token | DataType, args: list["Argument"]):
        super().__init__(category)
        self.__category = category.value
        self.__data_type = DataType(data_type)
        self.__args = args

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        data_type = self.__data_type.instantiate(template_type)
        args = [a.instantiate_templated_types(template_type) for a in self.__args]
        return NodeConstructor(self.token, data_type, args)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        for arg in self.__args:
            arg.init()

    def _init(self, valid_types: set[DataType]) -> None:
        # Check arguments are valid
        for arg in self.__args:
            if arg.name is None:
                raise CompileError("Unnamed argument in node constructors.", self.token)

    @property
    def _data_type(self) -> DataType:
        return self.__data_type

    def _evaluate(self) -> mx_utils.Node:
        node = mx_utils.create_node(self.__category, self.data_type)
        for arg in self.__args:
            node.set_input(arg.name, arg.evaluate())
        return node

    def __str__(self) -> str:
        return f'{{"{self.__category}", {self.__data_type}: {format_args(self.__args, with_names=True)}}}'
