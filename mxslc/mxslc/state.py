from __future__ import annotations

from . import utils
from .CompileError import CompileError
from .DataType import DataType
from .Function import Function
from .mx_classes import Node, NodeGraph, Output
from .Token import Token, IdentifierToken
from .document import is_document, get_document
from .scan import as_token


# TODO there is currently a bug, in the following scenario:
"""
float x = 3.0;

float add_x(float in)
{
    return in + x;
}

void main()
{
    float x = 5.0;
    float y = add_x(1.0);
}
"""
# TODO cont.: y will evaluate to 6.0 instead of 4.0 because x is finding the 5.0 before 3.0 when ascending the state hierarchy.


class State:
    def __init__(self, parent: State = None, node_graph: NodeGraph = None):
        self.__parent = parent
        self.__node_def = node_graph.node_def if node_graph else None
        self.__node_graph = node_graph
        self.__graph_nodes: dict[str, Node] = {}
        self.__implicit_args: dict[str, Node] = {}
        self.__implicit_outs: dict[str, Output] = {}
        self.__functions: list[Function] = []

        if self.__in_node_graph:
            for input_ in self.__node_def.inputs:
                dot_node = self.add_unnamed_node("dot", input_.data_type)
                dot_node.add_interface_input("in", input_.data_type, input_.name)
                self.add_node(IdentifierToken(input_.name), dot_node)

    def __str__(self) -> str:
        if self.__in_node_graph:
            return self.__node_graph.name
        else:
            return "document"

    #
    # properties
    #

    @property
    def parent(self) -> State:
        return self.__parent

    @property
    def implicit_outputs(self) -> dict[str, Output]:
        return self.__implicit_outs

    @property
    def __in_node_graph(self) -> bool:
        return self.__node_graph is not None

    #
    # add/get/set nodes
    #

    def add_node(self, identifier: Token, node: Node) -> None:
        # check node has the correct parent
        if self.__in_node_graph:
            assert node.parent == self.__node_graph
        else:
            assert is_document(node.parent)

        # check node is not somehow already stored in state
        assert node not in self.__graph_nodes.values()

        # fail if variable name already exists
        name = identifier.lexeme
        if name in self.__graph_nodes:
            raise CompileError(f"Variable name '{name}' already exists.", identifier)

        # store node in state
        self.__graph_nodes[name] = node
        node.name = name

    def get_node(self, identifier: Token) -> Node:
        name = identifier.lexeme

        # -- in document
        if not self.__in_node_graph:
            # return node if it exists
            if name in self.__graph_nodes:
                return self.__graph_nodes[name]
            # fail if node does not exist
            raise CompileError(f"Variable '{name}' does not exist.", identifier)

        # -- in nodegraph
        # return node if it exists
        if name in self.__graph_nodes:
            return self.__graph_nodes[name]

        # return locally updated global node
        if name in self.__implicit_outs:
            return self.__implicit_outs[name].value

        # get node from parent (recursive)
        input_node = self.parent.get_node(identifier)

        # create implicit argument
        if name not in self.__implicit_args:
            # create nodedef input based on global node
            nd_input = self.__node_def.add_input(name, data_type=input_node.data_type)
            # create dot node to make nodedef input accessible in the nodegraph
            dot_node = self.add_unnamed_node("dot", nd_input.data_type)
            dot_node.add_interface_input("in", nd_input.data_type, nd_input.name)
            self.__implicit_args[name] = dot_node

        return self.__implicit_args[name]

    def set_node(self, identifier: Token, node: Node) -> None:
        name = identifier.lexeme

        # -- in document
        if not self.__in_node_graph:
            # set node if it exists
            if name in self.__graph_nodes:
                self.__graph_nodes[name] = node
                node.name = name
                return
            # fail if node does not exist
            raise CompileError(f"Variable '{name}' does not exist.", identifier)

        # -- in nodegraph
        # set node if it exists
        if name in self.__graph_nodes:
            self.__graph_nodes[name] = node
            node.name = name
            return

        # create nodedef output based on local node
        if name in self.__implicit_outs:
            output_name = self.__implicit_outs[name].name
        else:
            output_name = self.__node_def.add_output(name, data_type=node.data_type).name

        # create nodegraph output based on local node
        self.__implicit_outs[name] = self.__node_graph.set_output(output_name, node)

    #
    # add/get functions
    #

    def add_function(self, func: Function) -> None:
        assert func not in self.__functions
        self.__functions.append(func)

    def get_function(self, identifier: Token, template_type: DataType = None, valid_types: set[DataType] = None, args: list["Argument"] = None) -> Function:
        matching_funcs = self.__get_functions(identifier, template_type, valid_types, args)
        if len(matching_funcs) == 0:
            raise CompileError(f"Function signature '{utils.function_signature_string(valid_types, identifier.lexeme, template_type, args)}' does not exist.", identifier)
        elif len(matching_funcs) == 1:
            return matching_funcs[0]
        else:
            return_types = {f.return_type for f in matching_funcs}
            raise CompileError(f"Function signature '{utils.function_signature_string(return_types, identifier.lexeme, template_type, args)}' is ambiguous.", identifier)

    def get_function_parameter_types(self, valid_types: set[DataType], identifier: Token, template_type: DataType, param_index: int | str) -> set[DataType]:
        matching_funcs = self.__get_functions(identifier, template_type, valid_types)
        return {
            f.parameters[param_index].data_type
            for f
            in matching_funcs
            if param_index in f.parameters
        }

    def __get_functions(self, identifier: Token, template_type: DataType = None, valid_types: set[DataType] = None, args: list["Argument"] = None) -> list[Function]:
        matching_funcs = [
            f
            for f
            in self.__functions
            if f.is_match(identifier.lexeme, template_type, valid_types, args)
        ]
        if len(matching_funcs) == 0:
            if self.__in_node_graph:
                return self.parent.__get_functions(identifier, template_type, valid_types, args)
            else:
                return []
        else:
            return matching_funcs

    #
    # add unnamed node
    #

    def add_unnamed_node(self, category: str, data_type: DataType) -> Node:
        if self.__in_node_graph:
            return self.__node_graph.add_node(category, data_type)
        else:
            return get_document().add_node(category, data_type)


_state = State()


def enter_node_graph(node_graph: NodeGraph) -> None:
    global _state
    _state = State(_state, node_graph)


def exit_node_graph() -> dict[str, Output]:
    global _state
    child_state = _state
    _state = _state.parent
    return child_state.implicit_outputs


def clear() -> None:
    global _state
    _state = State()


def add_node(identifier: str | Token, node: Node) -> None:
    _state.add_node(as_token(identifier), node)


def get_node(identifier: str | Token) -> Node:
    return _state.get_node(as_token(identifier))


def set_node(identifier: str | Token, node: Node) -> None:
    _state.set_node(as_token(identifier), node)


def is_node(identifier: str | Token) -> bool:
    try:
        get_node(identifier)
        return True
    except CompileError:
        return False


def add_function(func: Function) -> None:
    _state.add_function(func)


def get_function(identifier: str | Token, template_type: DataType = None, valid_types: set[DataType] = None, args: list["Argument"] = None) -> Function:
    return _state.get_function(as_token(identifier), template_type, valid_types, args)


def get_function_parameter_types(valid_types: set[DataType], identifier: str | Token, template_type: DataType, param_index: int | str) -> set[DataType]:
    return _state.get_function_parameter_types(valid_types, as_token(identifier), template_type, param_index)


def is_function(identifier: str | Token) -> bool:
    try:
        get_function(identifier)
        return True
    except CompileError:
        return False


def add_unnamed_node(category: str, data_type: DataType) -> Node:
    return _state.add_unnamed_node(category, data_type)
