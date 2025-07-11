from .. import node_utils
from ..DataType import DataType, INTEGER, FLOAT, MULTI_ELEM_TYPES
from ..Expressions import Expression
from ..mx_wrapper import Node


class IndexingExpression(Expression):
    def __init__(self, expr: Expression, indexer: Expression):
        super().__init__(indexer.token)
        self.__expr = expr
        self.__indexer = indexer

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        expr = self.__expr.instantiate_templated_types(template_type)
        indexer = self.__indexer.instantiate_templated_types(template_type)
        return IndexingExpression(expr, indexer)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        self.__expr.init(MULTI_ELEM_TYPES)
        self.__indexer.init(INTEGER)

    @property
    def _data_type(self) -> DataType:
        return FLOAT

    def _evaluate(self) -> Node:
        index = self.__indexer.evaluate()
        value = self.__expr.evaluate()
        return node_utils.extract(value, index)

    def __str__(self) -> str:
        return f"{self.__expr}[{self.__indexer}]"
