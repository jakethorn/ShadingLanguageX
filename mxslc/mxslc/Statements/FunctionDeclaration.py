from collections.abc import Collection

from . import Statement
from .. import state
from ..CompileError import CompileError
from ..DataType import DataType
from ..Expressions import Expression
from ..Function import Function, create_function
from ..Parameter import Parameter, ParameterList
from ..Token import Token


class FunctionDeclaration(Statement):
    def __init__(self,
                 is_inline: bool,
                 return_type: Token | DataType,
                 identifier: Token,
                 template_types: Collection[Token] | Collection[DataType],
                 params: ParameterList | list[Parameter],
                 body: list[Statement],
                 return_expr: Expression):
        super().__init__()
        self.__is_inline = is_inline
        self.__return_type = DataType(return_type)
        self.__identifier = identifier
        self.__template_types = {DataType(t) for t in template_types}
        self.__params = ParameterList(params)
        self.__body = body
        self.__return_expr = return_expr

        self.__funcs: list[Function] = []
        if len(template_types) == 0:
            func = create_function(is_inline, self.__return_type, identifier, None, self.__params, body, return_expr)
            self.__funcs.append(func)
        else:
            # duplicate function definition for each template type
            for template_type in self.__template_types:
                concrete_return_type = self.__return_type.instantiate(template_type)
                concrete_params = self.__params.instantiate_templated_parameters(template_type)
                concrete_body = [s.instantiate_templated_types(template_type) for s in body]
                concrete_return_expr = return_expr.instantiate_templated_types(template_type)
                func = create_function(is_inline, concrete_return_type, identifier, template_type, concrete_params, concrete_body, concrete_return_expr)
                self.__funcs.append(func)

    def instantiate_templated_types(self, template_type: DataType) -> Statement:
        if self.__template_types:
            raise CompileError("Cannot declare nested templated functions.", self.__identifier)
        return FunctionDeclaration(self.__is_inline, self.__return_type, self.__identifier, {template_type}, self.__params, self.__body, self.__return_expr)

    def execute(self) -> None:
        for func in sorted(self.__funcs):
            func.initialise()
            func.add_attributes(self._attribs)
            state.add_function(func)
