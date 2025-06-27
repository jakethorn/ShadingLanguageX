from __future__ import annotations

from pathlib import Path

from . import state
from .Argument import Argument
from .DataType import DataType
from .Expressions import Expression
from .Expressions.LiteralExpression import NullExpression
from .Keyword import Keyword
from .Parameter import ParameterList, Parameter
from .Token import Token, IdentifierToken
from .document import get_document
from .mx_classes import NodeDef, Node, Output


class Function:
    def __init__(self, return_type: DataType, identifier: Token, template_type: DataType | None, params: ParameterList, body: list["Statement"], return_expr: Expression):
        self.__return_type = return_type
        self.__name = identifier.lexeme
        self.__template_type = template_type
        self.__params = params
        self.__body = body
        self.__return_expr = return_expr
        self.__file = identifier.file
        self.__line = identifier.line

        self.__node_def: NodeDef | None = None
        self.__implicit_outs: dict[str, Output] = {}

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

    def initialise(self) -> None:
        self.__create_node_def()
        self.__create_node_graph()

    def is_match(self, name: str, template_type: DataType = None, return_types: set[DataType] = None, args: list[Argument] = None) -> bool:
        if self.__name != name:
            return False

        if template_type:
            if template_type != self.__template_type:
                return False

        if return_types:
            if self.__return_type not in return_types:
                return False

        if args:
            try:
                satisfied_params = [self.__params[a] for a in args]
            except IndexError:
                return False
            for param in self.__params:
                if param not in satisfied_params and param.default_value is None:
                    return False

        return True

    def invoke(self, args: list[Argument]) -> Node:
        return self.__call_node_def(args)

    @staticmethod
    def from_node_def(node_def: NodeDef) -> Function:
        return_type = node_def.output.data_type
        identifier = IdentifierToken(node_def.node_string)
        template_keyword = node_def.name.split("_")[-1]
        if template_keyword in Keyword.DATA_TYPES():
            template_type = DataType(template_keyword)
        else:
            template_type = None
        params = ParameterList()
        for input_ in node_def.inputs:
            param_identifier = IdentifierToken(input_.name)
            params += Parameter(param_identifier, input_.data_type, NullExpression())
        params.init_default_values()
        func = Function(return_type, identifier, template_type, params, None, None)
        func.__node_def = node_def
        return func

    def __create_node_def(self) -> None:
        self.__node_def = get_document().add_node_def(f"ND_{self.__name}", self.__return_type, self.__name)
        for param in self.__params:
            self.__node_def.add_input(param.name, param.data_type.default())

    def __create_node_graph(self) -> None:
        node_graph = get_document().add_node_graph_from_def(self.__node_def)
        state.enter_node_graph(node_graph)
        for stmt in self.__body:
            stmt.execute()
        retval = self.__return_expr.init_evaluate(self.__return_type)
        node_graph.add_output("out", retval)
        self.__implicit_outs = state.exit_node_graph()

    def __call_node_def(self, args: list[Argument]) -> Node:
        assert self.__node_def is not None
        node = state.add_unnamed_node(self.__name, self.__return_type)
        func_args = self.__combine_with_default_params(args)
        for nd_input in self.__node_def.inputs:
            if nd_input.name in func_args:
                node.add_input(nd_input.name, func_args[nd_input.name])
            else:
                node.add_input(nd_input.name, state.get_node(nd_input.name))

        if len(self.__implicit_outs) > 0:
            node.source.setType("multioutput")

            for name, output in self.__implicit_outs.items():
                dot_node = state.add_unnamed_node("dot", output.data_type)
                dot_node.add_input("in", output).value = node
                state.set_node(name, dot_node)

            dot_node = state.add_unnamed_node("dot", self.__node_def.output.data_type)
            dot_node.add_input("in", self.__node_def.output).value = node

            return dot_node
        else:
            return node

    def __combine_with_default_params(self, args: list[Argument]) -> dict[str, Node]:
        pairs: dict[str, Expression] = {p.name: p.default_value for p in self.__params}
        for arg in args:
            pairs[self.__params[arg].name] = arg.expression
        return {name: expr.evaluate() for name, expr in pairs.items()}
