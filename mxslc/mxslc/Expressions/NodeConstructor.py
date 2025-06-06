from . import Expression
from .. import mtlx
from ..CompileError import CompileError
from ..Keyword import DataType
from ..Token import Token


class NodeConstructor(Expression):
    def __init__(self, category: Token, data_type: Token, args: list["Argument"]):
        super().__init__(category)
        self.__category = category.value
        self.__data_type = DataType(data_type.type)
        self.__args = args

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        data_type_token = Token(data_type if self.__data_type is DataType.T else self.__data_type)
        args = [a.instantiate_templated_types(data_type) for a in self.__args]
        return NodeConstructor(self.token, data_type_token, args)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        for arg in self.__args:
            arg.expression.init()

    def _init(self, valid_types: list[DataType]) -> None:
        # Check arguments are valid
        for arg in self.__args:
            if arg.name is None:
                raise CompileError("Unnamed argument in node constructors.", self.token)

    @property
    def _data_type(self) -> DataType:
        return self.__data_type

    def _evaluate(self) -> mtlx.Node:
        node = mtlx.create_node(self.__category, self.data_type)
        for arg in self.__args:
            node.set_input(arg.name, arg.evaluate())
        return node
