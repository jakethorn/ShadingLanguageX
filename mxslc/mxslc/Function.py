from pathlib import Path

from . import mtlx, state
from .Argument import Argument
from .DataType import DataType
from .Expressions import Expression
from .Parameter import ParameterList, Parameter
from .Token import Token


class Function:
    def __init__(self, return_type: DataType, name: Token, params: ParameterList, body: list["Statement"], return_expr: Expression):
        self.__return_type = return_type
        self.__name = name.lexeme
        self.__params = params
        self.__body = body
        self.__return_expr = return_expr
        self.__file = name.file
        self.__line = name.line

    @property
    def return_type(self) -> DataType:
        return self.__return_type

    @property
    def params(self) -> ParameterList:
        return self.__params

    @property
    def file(self) -> Path:
        return self.__file

    @property
    def line(self) -> int:
        return self.__line

    def is_match(self, name: str, template_type: DataType = None, return_types: list[DataType] = None, args: list[Argument] = None) -> bool:
        if self.__name != name:
            return False

        if template_type is not None:
            if self.return_type.is_templated and self.return_type != template_type:
                return False

            for param in self.__params:
                if param.data_type.is_templated and param.data_type != template_type:
                    return False

        if return_types is not None:
            if self.__return_type not in return_types:
                return False

        if args is not None:
            if len(args) > len(self.__params):
                return False

            for arg in args:
                if arg.is_named and arg.name not in self.__params.names:
                    return False

            satisfied_params = []
            for i, arg in enumerate(args):
                if arg.is_positional:
                    param = self.__params[i]
                    if arg.data_type != param.data_type:
                        return False
                    satisfied_params.append(param)
                else:
                    param = self.__params[arg.name]
                    if arg.data_type != param.data_type:
                        return False
                    satisfied_params.append(param)

            for param in self.__params:
                if param not in satisfied_params and param.default_value is None:
                    return False

        return True

    def invoke(self, args: list[Argument]) -> mtlx.Node:
        args = self.__sort_args(args)

        arg_nodes = []
        for param, arg in zip(self.__params, args):
            arg_node = arg.init_evaluate(param.data_type)
            arg_nodes.append(arg_node)

        state.enter_scope(self.__name)

        for param, arg_node in zip(self.__params, arg_nodes):
            state.add_node(param.name_token, arg_node)

        for statement in self.__body:
            statement.execute()

        retval = self.__return_expr.init_evaluate(self.__return_type)

        state.exit_scope()

        return retval

    def __sort_args(self, args: list[Argument]) -> list[Expression]:
        pairs: dict[Parameter, Expression] = {}
        for param in self.__params:
            pairs[param] = param.default_value
        for i, arg in enumerate(args):
            if arg.is_positional:
                pairs[self.__params[i]] = arg.expression
            else:
                pairs[self.__params[arg.name]] = arg.expression
        assert all([e is not None for e in pairs.values()])
        return list(pairs.values())
