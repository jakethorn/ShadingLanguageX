from . import Expression
from .. import mx_utils
from ..DataType import DataType, FLOAT, MULTI_ELEM_TYPES
from ..Token import Token


class ConstructorCall(Expression):
    def __init__(self, data_type: Token, args: list["Argument"]):
        super().__init__(data_type)
        self.__data_type = DataType(data_type)
        self.__args = args

    def instantiate_templated_types(self, template_type: DataType) -> Expression:
        data_type = self.__data_type.instantiate(template_type).as_token
        args = [a.instantiate_templated_types(template_type) for a in self.__args]
        return ConstructorCall(data_type, args)

    def _init_subexpr(self, valid_types: set[DataType]) -> None:
        if len(self.__args) == 1:
            self.__args[0].init()
        if len(self.__args) > 1:
            for arg in self.__args:
                arg.init({FLOAT} | MULTI_ELEM_TYPES)

    @property
    def _data_type(self) -> DataType:
        return self.__data_type

    def _evaluate(self) -> mx_utils.Node:
        if len(self.__args) == 0:
            return self.__constant_node()
        elif len(self.__args) == 1:
            return self.__convert_node()
        else:
            return self.__combine_node()

    def __constant_node(self) -> mx_utils.Node:
        return mx_utils.constant(self.data_type.zeros())

    def __convert_node(self) -> mx_utils.Node:
        return mx_utils.convert(self.__args[0].evaluate(), self.data_type)

    def __combine_node(self) -> mx_utils.Node:
        channels = []
        # fill channels with args
        for arg in self.__args:
            new_channels = mx_utils.extract_all(arg.evaluate())
            for new_channel in new_channels:
                channels.append(new_channel)
                if len(channels) == self.data_size:
                    return mx_utils.combine(channels, self.data_type)
        # fill remaining channels (if any) with zeros
        while len(channels) < self.data_size:
            channels.append(mx_utils.constant(0.0))
        return mx_utils.combine(channels, self.data_type)
