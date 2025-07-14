from __future__ import annotations

from pathlib import Path

from . import state, node_utils
from .Argument import Argument
from .CompileError import CompileError
from .DataType import DataType, VOID
from .Expressions import Expression
from .Expressions.LiteralExpression import NullExpression
from .Keyword import Keyword
from .Parameter import ParameterList, Parameter
from .Token import Token, IdentifierToken
from .document import get_document
from .mx_wrapper import Node, NodeDef, Output, NodeGraph


class Function:
    def __init__(self,
                 return_type: DataType,
                 identifier: Token,
                 template_type: DataType | None,
                 params: ParameterList,
                 body: list["Statement"] | None,
                 return_expr: Expression | None):
        self.__return_type = return_type
        self.__identifier = identifier
        self.__template_type = template_type
        self.__params = params
        self.__body = body
        self.__return_expr = return_expr

        self.__node_def: NodeDef | None = None
        self.__node_graph: NodeGraph | None = None
        self.__implicit_outs: dict[str, Output] = {}

    @property
    def return_type(self) -> DataType:
        # stdlib funcs dont always have a nodegraph, but they always have a return type
        if self.__node_graph is None:
            return self.__return_type
        # node graphs without an "out" output are void functions
        if self.__node_graph.has_output("out"):
            return self.__node_graph.get_output("out").data_type
        else:
            return VOID

    @property
    def parameters(self) -> ParameterList:
        return self.__params

    @property
    def name(self) -> str:
        return self.__identifier.lexeme

    @property
    def file(self) -> Path:
        return self.__identifier.file

    @property
    def line(self) -> int:
        return self.__identifier.line

    def initialise(self) -> None:
        self.parameters.init_default_values()
        self.__create_node_def()
        self.__create_node_graph()
        # update output data type if auto keyword was used
        if self.__node_def.has_output("out"):
            output = self.__node_def.get_output("out")
            if output.data_type == Keyword.AUTO:
                output.data_type = self.return_type
                output.default = self.return_type.default()

    def is_match(self, name: str, template_type: DataType = None, return_types: set[DataType] = None, args: list[Argument] = None) -> bool:
        if self.name != name:
            return False
        if template_type:
            if template_type != self.__template_type:
                return False
        if return_types:
            if self.return_type not in return_types:
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

    def __lt__(self, other: Function) -> bool:
        return self.__node_def_name < other.__node_def_name

    def __str__(self) -> str:
        if self.__template_type:
            return f"{self.return_type} {self.name}<{self.__template_type}>({self.parameters})"
        else:
            return f"{self.return_type} {self.name}({self.parameters})"

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

    @property
    def __node_def_name(self) -> str:
        return f"ND_{self.name}" if self.__template_type is None else f"ND_{self.name}_{self.__template_type}"

    def __create_node_def(self) -> None:
        self.__node_def = get_document().add_node_def(self.__node_def_name, self.__return_type, self.name)
        for param in self.__params:
            self.__node_def.add_input(param.name, data_type=param.data_type)

    def __create_node_graph(self) -> None:
        self.__node_graph = get_document().add_node_graph_from_def(self.__node_def)
        state.enter_node_graph(self.__node_graph)
        for stmt in self.__body:
            stmt.execute()
        retval = self.__return_expr.init_evaluate(self.__return_type)
        self.__node_graph.add_output("out", retval)
        self.__implicit_outs = state.exit_node_graph()

    def __call_node_def(self, args: list[Argument]) -> Node:
        assert self.__node_def is not None
        # create node
        node = node_utils.create(self.name, self.return_type)
        # add inputs
        func_args = self.__combine_with_default_params(args)
        for nd_input in self.__node_def.inputs:
            if nd_input.name in func_args:
                node.add_input(nd_input.name, func_args[nd_input.name])
            else:
                node.add_input(nd_input.name, state.get_node(nd_input.name))
        # add outputs
        if self.__node_def.output_count == 0:
            raise CompileError("Invalid function. Functions must return a value or update a variable from an outer scope.", self.__identifier)
        if self.__node_def.output_count == 1:
            node.data_type = self.__node_def.output.data_type
        if self.__node_def.output_count > 1:
            node.data_type = "multioutput"
            for nd_output in self.__node_def.outputs:
                node_output = node.add_output(nd_output.name, data_type=nd_output.data_type)
                node_output.clear_value()
        # update outer scope variables and return value
        if self.__node_def.output_count == 1:
            if len(self.__implicit_outs) == 1:
                name = list(self.__implicit_outs.keys())[0]
                state.set_node(name, node)
            return node
        else:
            for name, ng_output in self.__implicit_outs.items():
                node_output = node.get_output(ng_output.name)
                dot_node = node_utils.dot(node_output)
                state.set_node(name, dot_node)
            if self.__node_def.output_count == len(self.__implicit_outs):
                return node
            else:
                dot_node = node_utils.dot(node.output)
                return dot_node

    def __combine_with_default_params(self, args: list[Argument]) -> dict[str, Node]:
        pairs: dict[str, Expression] = {p.name: p.default_value for p in self.__params}
        for arg in args:
            pairs[self.__params[arg].name] = arg.expression
        return {name: expr.evaluate() for name, expr in pairs.items()}
