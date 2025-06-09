from . import Statement
from .. import state
from ..CompileError import CompileError
from ..DataType import DataType
from ..Function import Function
from ..Parameter import Parameter, ParameterList
from ..Token import Token


class FunctionDeclaration(Statement):
    def __init__(self, return_type: DataType, identifier: Token, template_types: set[DataType], params: ParameterList, body: list[Statement], return_expr: "Expression"):
        self.__return_type = return_type
        self.__identifier = identifier
        self.__template_types = template_types
        self.__params = params
        self.__body = body
        self.__return_expr = return_expr

        self.__funcs: list[Function] = []
        if len(template_types) == 0:
            func = Function(return_type, identifier, params, body, return_expr)
            self.__funcs.append(func)
        else:
            for template_type in template_types:
                concrete_return_type = return_type.instantiate(template_type)
                concrete_params = params.instantiate_templated_parameters(template_type)
                concrete_body = [s.instantiate_templated_types(template_type) for s in body]
                concrete_return_expr = return_expr.instantiate_templated_types(template_type)
                func = Function(concrete_return_type, identifier, concrete_params, concrete_body, concrete_return_expr)
                self.__funcs.append(func)

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        if self.__template_types:
            raise CompileError("Cannot declare nested templated functions.", self.__identifier)
        return FunctionDeclaration(self.__return_type, self.__identifier, {template_type}, self.__params, self.__body, self.__return_expr)

    def execute(self) -> None:
        for func in self.__funcs:
            _init_parameter_default_values(func)
            state.add_function(func)


def _init_parameter_default_values(func: Function) -> None:
    for param in func.parameters:
        if param.default_value:
            param.default_value.init(param.data_type)
