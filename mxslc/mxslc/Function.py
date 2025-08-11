from __future__ import annotations

from abc import ABC, abstractmethod
from pathlib import Path
from typing import Any

from . import state, node_utils
from .Argument import Argument
from .Attribute import Attribute
from .CompileError import CompileError
from .DataType import DataType, VOID
from .Expressions import Expression, IdentifierExpression
from .Expressions.LiteralExpression import NullExpression
from .Keyword import Keyword
from .Parameter import ParameterList, Parameter
from .Token import Token, IdentifierToken
from .document import get_document
from .mx_wrapper import Node, NodeDef, Output, NodeGraph

Statement = Any


def create_function(is_inline: bool, *args) -> Function:
    if is_inline:
        return InlineFunction(*args)
    else:
        return NodeGraphFunction(*args)


class Function(ABC):
    def __init__(self,
                 return_type: DataType,
                 identifier: Token,
                 template_type: DataType | None,
                 params: ParameterList,
                 body: list[Statement] | None,
                 return_expr: Expression | None):
        self._return_type = return_type
        self._identifier = identifier
        self._template_type = template_type
        self._params = params
        self._body = body
        self._return_expr = return_expr

    @property
    @abstractmethod
    def return_type(self) -> DataType:
        ...

    @property
    def name(self) -> str:
        return self._identifier.lexeme

    @property
    @abstractmethod
    def fullname(self) -> str:
        ...

    @property
    def parameters(self) -> ParameterList:
        return self._params

    @property
    def file(self) -> Path:
        return self._identifier.file

    @property
    def line(self) -> int:
        return self._identifier.line

    @property
    def _in_params(self) -> ParameterList:
        return self._params.ins

    @property
    def _out_params(self) -> ParameterList:
        return self._params.outs

    @abstractmethod
    def initialise(self) -> None:
        ...

    @abstractmethod
    def add_attributes(self, attribs: list[Attribute]) -> None:
        ...

    @abstractmethod
    def invoke(self, args: list[Argument]) -> Node:
        ...

    def is_match(self, name: str, template_type: DataType = None, return_types: set[DataType] = None, args: list[Argument] = None, strict_args=True) -> bool:
        if self.name != name:
            return False
        if template_type:
            if template_type != self._template_type:
                return False
        if return_types:
            if self.return_type not in return_types:
                return False
        if args:
            try:
                satisfied_params = [self._params[a] for a in args]
            except IndexError:
                return False
            if strict_args:
                for param in self._params:
                    if param not in satisfied_params and param.default_value is None:
                        return False
        return True

    def __lt__(self, other: Function) -> bool:
        return self.fullname < other.fullname

    def __str__(self) -> str:
        if self._template_type:
            return f"{self.return_type} {self.name}<{self._template_type}>({self.parameters})"
        else:
            return f"{self.return_type} {self.name}({self.parameters})"

    def _sort_arguments(self, args: list[Argument]) -> dict[str, Expression]:
        pairs: dict[str, Expression] = {p.name: p.default_value for p in self._params}
        for arg in args:
            pairs[self._params[arg].name] = arg.expression
        return pairs

    def _evaluate_arguments(self, args: dict[str, Expression]) -> dict[str, Node]:
        return {name: expr.evaluate() for name, expr in args.items()}


class InlineFunction(Function):
    @property
    def return_type(self) -> DataType:
        return self._return_type

    @property
    def fullname(self) -> str:
        if self._template_type:
            return f"{self.name}_{self._template_type}"
        else:
            return self.name

    def initialise(self) -> None:
        self.parameters.init_default_values()

    def add_attributes(self, attribs: list[Attribute]) -> None:
        if len(attribs) > 0:
            raise CompileError("Attributes cannot be defined above an inline function.", self._identifier)

    def invoke(self, args: list[Argument]) -> Node:
        func_args = self._sort_arguments(args)
        func_arg_nodes = self._evaluate_arguments(func_args)
        state.enter_inline()
        for name, node in func_arg_nodes.items():
            state.add_node(name, node)
        for stmt in self._body:
            stmt.execute()
        out_nodes = {p.name: state.get_node(p.name) for p in self._out_params}
        retval = self._return_expr.init_evaluate(self._return_type)
        state.exit_inline()
        for name, node in out_nodes.items():
            arg_expr = func_args[name]
            if not isinstance(arg_expr, IdentifierExpression):
                raise CompileError(f"Invalid argument being passed to out parameter: {arg_expr}.", arg_expr.token)
            state.set_node(arg_expr.identifier, node)
        return retval


class NodeGraphFunction(Function):
    def __init__(self,
                 return_type: DataType,
                 identifier: Token,
                 template_type: DataType | None,
                 params: ParameterList,
                 body: list[Statement] | None,
                 return_expr: Expression | None):
        super().__init__(return_type, identifier, template_type, params, body, return_expr)
        self.__node_def: NodeDef | None = None
        self.__node_graph: NodeGraph | None = None
        self.__implicit_outs: dict[str, Output] = {}

    @property
    def return_type(self) -> DataType:
        # stdlib funcs dont always have a nodegraph, but they always have a return type
        if self.__node_graph is None:
            return self._return_type
        # node graphs without an "out" output are void functions
        if self.__node_graph.has_output("out"):
            return self.__node_graph.get_output("out").data_type
        else:
            return VOID

    @property
    def fullname(self) -> str:
        return f"ND_{self.name}" if self._template_type is None else f"ND_{self.name}_{self._template_type}"

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

    def add_attributes(self, attribs: list[Attribute]) -> None:
        for attrib in attribs:
            if attrib.child is None:
                self.__node_def.set_attribute(attrib.name, attrib.value)
            else:
                child = self.__node_def.get_child(attrib.child)
                child.set_attribute(attrib.name, attrib.value)

    def invoke(self, args: list[Argument]) -> Node:
        return self.__call_node_def(args)

    @staticmethod
    def from_node_def(node_def: NodeDef) -> Function:
        if node_def.output_count == 1:
            return_type = node_def.output.data_type
        else:
            return_type = VOID
        identifier = IdentifierToken(node_def.node_string)
        template_keyword = node_def.name.split("_")[-1]
        if template_keyword in Keyword.DATA_TYPES():
            template_type = DataType(template_keyword)
        else:
            template_type = None
        params = ParameterList()
        if node_def.output_count > 1:
            for output in node_def.outputs:
                param_identifier = IdentifierToken(output.name)
                params += Parameter(param_identifier, output.data_type, is_out=True)
        for input_ in node_def.inputs:
            param_identifier = IdentifierToken(input_.name)
            params += Parameter(param_identifier, input_.data_type, NullExpression())
        params.init_default_values()
        func = NodeGraphFunction(return_type, identifier, template_type, params, None, None)
        func.__node_def = node_def
        return func

    def __create_node_def(self) -> None:
        self.__node_def = get_document().add_node_def(self.fullname, self._return_type, self.name)
        for param in self._in_params:
            self.__node_def.add_input(param.name, data_type=param.data_type)
        for param in self._out_params:
            self.__node_def.add_output(param.name, data_type=param.data_type)

    def __create_node_graph(self) -> None:
        self.__node_graph = get_document().add_node_graph_from_def(self.__node_def)
        for param in self._out_params:
            self.__node_graph.add_output(param.name, data_type=param.data_type)
        state.enter_node_graph(self.__node_graph)
        for param in self._out_params:
            state.add_node(param.name, node_utils.constant(param.data_type.default()))
        for stmt in self._body:
            stmt.execute()
        retval = self._return_expr.init_evaluate(self._return_type)
        self.__node_graph.add_output("out", retval)
        for param in self._out_params:
            self.__node_graph.set_output(param.name, state.get_node(param.name))
        self.__implicit_outs = state.exit_node_graph()

    def __call_node_def(self, args: list[Argument]) -> Node:
        assert self.__node_def is not None
        func_args = self._sort_arguments(args)
        func_arg_nodes = self._evaluate_arguments(func_args)
        node = node_utils.create(self.name, self.return_type)
        # add inputs to node
        for nd_input in self.__node_def.inputs:
            if nd_input.name in func_arg_nodes:
                node.add_input(nd_input.name, func_arg_nodes[nd_input.name])
            else:
                node.add_input(nd_input.name, state.get_node(nd_input.name))
        # add outputs to node
        if self.__node_def.output_count == 0:
            raise CompileError("Invalid function. Functions must return a value or update a variable from an outer scope.", self._identifier)
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
            if len(self._out_params) == 1:
                arg_expr = func_args[self._out_params[0].name]
                if not isinstance(arg_expr, IdentifierExpression):
                    raise CompileError(f"Invalid argument being passed to out parameter: {arg_expr}.", arg_expr.token)
                state.set_node(arg_expr.identifier, node)
            return node
        else:
            for name, ng_output in self.__implicit_outs.items():
                node_output = node.get_output(ng_output.name)
                dot_node = node_utils.dot(node_output)
                state.set_node(name, dot_node)
            for param in self._out_params:
                arg_expr = func_args[param.name]
                if not isinstance(arg_expr, IdentifierExpression):
                    raise CompileError(f"Invalid argument being passed to out parameter: {arg_expr}.", arg_expr.token)
                node_output = node.get_output(param.name)
                dot_node = node_utils.dot(node_output)
                state.set_node(arg_expr.identifier, dot_node)
            if self.__node_def.output_count == len(self.__implicit_outs) + len(self._out_params):
                return node
            else:
                dot_node = node_utils.dot(node.output)
                return dot_node
