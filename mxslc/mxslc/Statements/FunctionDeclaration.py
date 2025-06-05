from . import Statement
from .. import mtlx, state
from ..Argument import Argument
from ..Expressions import Expression
from ..Keyword import DataType
from ..Parameter import Parameter, ParameterList
from ..Token import Token

# TODO change params to a ParameterList instead of list[Parameter]
# TODO change data_type name to "return_type"

class FunctionDeclaration(Statement):
    def __init__(self, data_type: Token, identifier: Token, params: list[Parameter], body: list[Statement], return_expr: Expression):
        self.file = identifier.file
        self.line = identifier.line

        self.data_type = DataType(data_type.type)
        self.identifier = identifier
        self.name = identifier.lexeme
        self.params = params
        self.body = body
        self.return_expr = return_expr

    def execute(self) -> None:
        state.add_function(self)

    def is_match(self, name: str, data_types: list[DataType] = None, args: list[Argument] = None) -> bool:
        if self.name != name:
            return False

        if data_types is not None:
            if self.data_type not in data_types:
                return False

        if args is not None:
            if len(args) > len(self.params):
                return False

            for arg in args:
                if arg.is_named and arg.name not in [p.name for p in self.params]:
                    return False

            satisfied_params = []
            param_index = 0
            for arg in args:
                if arg.is_positional:
                    param = self.params[param_index]
                    if arg.data_type != param.data_type:
                        return False
                    satisfied_params.append(param)
                    param_index += 1
                else:
                    param = ParameterList(self.params)[arg.name]
                    if arg.data_type != param.data_type:
                        return False
                    satisfied_params.append(param)

            for param in self.params:
                if param not in satisfied_params and param.default_value is None:
                    return False

        return True

    def invoke(self, args: list[Argument]) -> mtlx.Node:
        args = self.__sort_args(args)

        # evaluate arguments before entering function scope because they might reference variable from the calling scope
        arg_nodes = []
        for param, arg in zip(self.params, args):
            arg_node = arg.init_evaluate(param.data_type)
            arg_nodes.append(arg_node)

        # enter scope
        state.enter_scope(self.name)

        # now add arguments to state
        for param, arg_node in zip(self.params, arg_nodes):
            state.add_node(param.name_token, arg_node)

        for statement in self.body:
            statement.execute()

        retval = self.return_expr.init_evaluate(self.data_type)

        state.exit_scope()

        return retval


    def __sort_args(self, args: list[Argument]) -> list[Expression]:
        pairs: dict[Parameter, Expression] = {}
        params = ParameterList(self.params)
        for param in params:
            pairs[param] = param.default_value
        for i, arg in enumerate(args):
            if arg.is_positional:
                pairs[params[i]] = arg.expression
            else:
                pairs[params[arg.name]] = arg.expression
        assert all([e is not None for e in pairs.values()])
        return list(pairs.values())
