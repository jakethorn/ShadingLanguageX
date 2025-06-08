from pathlib import Path

from . import mx_utils, state
from .Argument import Argument
from .DataType import DataType
from .Expressions import Expression
from .Parameter import ParameterList, Parameter
from .Token import Token


class Function:
    def __init__(self, return_type: DataType, identifier: Token, params: ParameterList, body: list["Statement"], return_expr: Expression):
        self.__return_type = return_type
        self.__name = identifier.lexeme
        self.__params = params
        self.__body = body
        self.__return_expr = return_expr
        self.__file = identifier.file
        self.__line = identifier.line

    @property
    def return_type(self) -> DataType:
        return self.__return_type

    @property
    def parameters(self) -> ParameterList:
        return self.__params

    @property
    def file(self) -> Path:
        return self.__file

    @property
    def line(self) -> int:
        return self.__line

    def is_match(self, name: str, template_type: DataType = None, return_types: set[DataType] = None, args: list[Argument] = None) -> bool:
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
            try:
                satisfied_params = [self.__params[a] for a in args]
            except IndexError:
                return False
            for param in self.__params:
                if param not in satisfied_params and param.default_value is None:
                    return False

        return True

    def invoke(self, args: list[Argument]) -> mx_utils.Node:
        arg_nodes = [a.evaluate() for a in self.__sort_args(args)]

        state.enter_scope(self.__name)

        for param, arg_node in zip(self.__params, arg_nodes):
            state.add_node(param.identifier, arg_node)

        for statement in self.__body:
            statement.execute()

        retval = self.__return_expr.init_evaluate(self.__return_type)

        state.exit_scope()

        return retval

    def __sort_args(self, args: list[Argument]) -> list[Expression]:
        pairs: dict[Parameter, Expression] = {}
        for param in self.__params:
            pairs[param] = param.default_value
        for arg in args:
            pairs[self.__params[arg]] = arg.expression
        return list(pairs.values())
