from mxslc import mtlx
from mxslc.Expressions import Expression
from mxslc.Keyword import DataType, FLOAT, INTEGER, VECTOR_TYPES, COLOR_TYPES


class IndexingExpression(Expression):
    def __init__(self, expr: Expression, indexer: Expression):
        super().__init__(indexer.token)
        self.__expr = expr
        self.__indexer = indexer

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        self.__expr.init(VECTOR_TYPES + COLOR_TYPES)
        self.__indexer.init(INTEGER)

    @property
    def _data_type(self) -> DataType:
        return FLOAT

    def _evaluate(self) -> mtlx.Node:
        index = self.__indexer.evaluate()
        value = self.__expr.evaluate()
        return mtlx.extract(value, index)
