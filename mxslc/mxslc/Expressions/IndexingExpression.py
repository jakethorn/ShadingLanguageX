from mxslc import mx_utils
from mxslc.DataType import DataType, INTEGER, FLOAT, MULTI_ELEM_TYPES
from mxslc.Expressions import Expression


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

    def _evaluate(self) -> mx_utils.Node:
        index = self.__indexer.evaluate()
        value = self.__expr.evaluate()
        return mx_utils.extract(value, index)
