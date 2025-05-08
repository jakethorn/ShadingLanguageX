from . import Statement
from .. import mtlx, state
from ..Argument import Argument
from ..CompileError import CompileError
from ..Expressions import Expression
from ..Keyword import DataType
from ..Parameter import Parameter
from ..StandardLibrary import StandardLibrary
from ..Token import Token

# TODO change params to a ParameterList instead of list[Parameter]

class FunctionDeclaration(Statement):
    def __init__(self, data_type: Token, identifier: Token, params: list[Parameter], body: list[Statement], return_expr: Expression):
        self.file = identifier.file
        self.line = identifier.line

        self.data_type = DataType(data_type.type)
        self.name = identifier.lexeme
        self.params = params
        self.body = body
        self.return_expr = return_expr

    def execute(self) -> None:
        if self.name in StandardLibrary:
            raise CompileError(self.line, f"Function name '{self.name}' already exists in the standard library.")
        state.add_function(self)

    def invoke(self, args: list[Argument]) -> mtlx.Node:
        if len(self.params) != len(args):
            raise CompileError(self.line, f"Incorrect number of arguments for function '{self.name}'.")

        # evaluate arguments before entering function scope because they might reference variable from the calling scope
        arg_nodes = []
        for param, arg in zip(self.params, args):
            arg_node = arg.evaluate(param.data_types)
            arg_nodes.append(arg_node)

        # enter scope
        state.enter_scope(self.name)

        # now add arguments to state
        for param, arg_node in zip(self.params, arg_nodes):
            state.add_node(param.name_token, arg_node)

        for statement in self.body:
            statement.execute()

        retval = self.return_expr.evaluate(self.data_type)

        state.exit_scope()

        return retval
