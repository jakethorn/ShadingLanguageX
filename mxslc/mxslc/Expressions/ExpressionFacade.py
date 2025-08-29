from .Expression import Expression
from ..DataType import DataType
from ..mx_wrapper import Node, Uniform


class ExpressionFacade(Expression):
    def __init__(self, expr: Expression, str_: str):
        super().__init__(expr.token)
        self.__expr = expr
        self.__str = str_

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        return self.__expr.instantiate_templated_types(template_type)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.__expr._init_subexpr(valid_types)

    def _init(self, valid_types: set[DataType]) -> None:
        self.__expr._init(valid_types)

    @property
    def _data_type(self) -> DataType:
        return self.__expr._data_type

    @property
    def _value(self) -> Uniform | None:
        return self.__expr._value

    def _evaluate(self) -> Node:
        return self.__expr._evaluate()

    def __str__(self) -> str:
        return self.__str
