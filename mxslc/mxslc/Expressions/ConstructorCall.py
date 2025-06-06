from . import Expression
from .. import mtlx
from ..Keyword import DataType, FLOAT_TYPES
from ..Token import Token


class ConstructorCall(Expression):
    def __init__(self, data_type: Token, args: list["Argument"]):
        super().__init__(data_type)
        self.__data_type = DataType(data_type.type)
        self.__args = args

    def instantiate_templated_types(self, data_type: DataType) -> Expression:
        data_type_token = Token(data_type if self.__data_type is DataType.T else self.__data_type)
        args = [a.instantiate_templated_types(data_type) for a in self.__args]
        return ConstructorCall(data_type_token, args)

    def _init_subexpr(self, valid_types: list[DataType]) -> None:
        if len(self.__args) == 1:
            self.__args[0].expression.init()
        if len(self.__args) > 1:
            for arg in self.__args:
                arg.expression.init(FLOAT_TYPES)

    @property
    def _data_type(self) -> DataType:
        return self.__data_type

    def _evaluate(self) -> mtlx.Node:
        if len(self.__args) == 0:
            return self.__constant_node()
        elif len(self.__args) == 1:
            return self.__convert_node()
        else:
            return self.__combine_node()

    def __constant_node(self) -> mtlx.Node:
        return mtlx.constant(self.data_type.zeros())

    def __convert_node(self) -> mtlx.Node:
        return mtlx.convert(self.__args[0].evaluate(), self.data_type)

    def __combine_node(self) -> mtlx.Node:
        channels = []
        # fill channels with args
        for arg in self.__args:
            new_channels = mtlx.extract_all(arg.evaluate())
            for new_channel in new_channels:
                channels.append(new_channel)
                if len(channels) == self.data_size:
                    return mtlx.combine(channels, self.data_type)
        # fill remaining channels (if any) with zeros
        while len(channels) < self.data_size:
            channels.append(mtlx.constant(0.0))
        return mtlx.combine(channels, self.data_type)
