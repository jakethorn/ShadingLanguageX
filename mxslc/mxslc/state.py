from __future__ import annotations

from . import mx_utils, utils
from .CompileError import CompileError
from .DataType import DataType
from .Function import Function
from .Statements import ForLoop
from .Token import Token
from .scan import as_token


class State:
    def __init__(self, namespace: str, global_: State = None, parent: State = None):
        self.__namespace = namespace
        self.__global = global_ or self
        self.__parent = parent
        self.__nodes: dict[str, mx_utils.Node] = {}
        self.__functions: list[Function] = []

    @property
    def is_global(self) -> bool:
        return self == self.__global

    @property
    def global_(self) -> State:
        return self.__global

    @property
    def parent(self) -> State:
        return self.__parent

    def add_node(self, identifier: Token, node: mx_utils.Node) -> None:
        name = identifier.lexeme
        if name in self.__nodes:
            raise CompileError(f"Variable name '{name}' already exists.", identifier)
        assert node not in self.__nodes.values()
        self.__nodes[name] = node
        node.name = self.get_full_name(name)

    def get_node(self, identifier: Token) -> mx_utils.Node:
        name = identifier.lexeme
        if name in self.__nodes:
            return self.__nodes[name]
        if self.__namespace == ForLoop.NAMESPACE:
            try:
                return self.__parent.get_node(identifier)
            except CompileError:
                ...
        if name in self.__global.__nodes:
            return self.__global.__nodes[name]
        raise CompileError(f"Variable '{name}' does not exist.", identifier)

    def set_node(self, identifier: Token, node: mx_utils.Node) -> None:
        name = identifier.lexeme
        assert node not in self.__nodes.values()
        if name in self.__nodes:
            self.__nodes[name] = node
            node.name = self.get_full_name(name)
            return
        if self.__namespace == ForLoop.NAMESPACE:
            try:
                self.__parent.set_node(identifier, node)
                return
            except CompileError:
                ...
        # If I wanted to let developers set global variables I would do it here
        raise CompileError(f"Variable name '{name}' does not exist.", identifier)

    def clear(self) -> None:
        self.__nodes.clear()
        self.__functions.clear()

    def get_full_name(self, name: str) -> str:
        return f"{self.__namespace}__{name}"

    def add_function(self, func: Function) -> None:
        assert func not in self.__functions
        self.__functions.append(func)

    def get_function(self, identifier: Token, template_type: DataType = None, valid_types: set[DataType] = None, args: list["Argument"] = None) -> Function:
        matching_funcs = [
            f
            for f
            # TODO also search local functions
            in self.__global.__functions
            if f.is_match(identifier.lexeme, template_type, valid_types, args)
        ]
        if len(matching_funcs) == 0:
            raise CompileError(f"Function signature '{utils.function_signature_string(valid_types, identifier.lexeme, template_type, args)}' does not exist.", identifier)
        elif len(matching_funcs) == 1:
            return matching_funcs[0]
        else:
            return_types = {f.return_type for f in matching_funcs}
            raise CompileError(f"Function signature '{utils.function_signature_string(return_types, identifier.lexeme, template_type, args)}' is ambiguous.", identifier)

    def get_function_parameter_types(self, valid_types: set[DataType], identifier: Token, template_type: DataType, param_index: int | str) -> set[DataType]:
        matching_funcs = [
            f
            for f
            # TODO also search local functions
            in self.__global.__functions
            if f.is_match(identifier.lexeme, template_type, valid_types)
        ]
        return {
            f.parameters[param_index].data_type
            for f
            in matching_funcs
            if param_index in f.parameters
        }

    def __str__(self) -> str:
        output = ""
        output += "self: " + self.__namespace + "\n"
        output += "parent: " + (self.__parent.__namespace if self.__parent else "None") + "\n"
        output += "global: " + self.__global.__namespace + "\n"
        output += "----------------" + "\n"
        for name, node in self.__nodes.items():
            output += f"{name}: {node.data_type} {node.name}\n"
        return output


_state = State("global")


def add_node(identifier: Token, node: mx_utils.Node) -> None:
    _state.add_node(identifier, node)


def get_node(identifier: str | Token) -> mx_utils.Node:
    return _state.get_node(as_token(identifier))


def set_node(identifier: str | Token, node: mx_utils.Node) -> None:
    _state.set_node(as_token(identifier), node)


def is_node(identifier: str) -> bool:
    try:
        get_node(identifier)
        return True
    except CompileError:
        return False


def clear() -> None:
    global _state
    while not _state.is_global:
        _state = _state.parent
    _state.clear()


def add_function(func: Function) -> None:
    _state.add_function(func)


def get_function(identifier: str | Token, template_type: DataType = None, valid_types: set[DataType] = None, args: list["Argument"] = None) -> Function:
    return _state.get_function(as_token(identifier), template_type, valid_types, args)


def get_function_parameter_types(valid_types: set[DataType], identifier: str | Token, template_type: DataType, param_index: int | str) -> set[DataType]:
    return _state.get_function_parameter_types(valid_types, as_token(identifier), template_type, param_index)


def is_function(identifier: str) -> bool:
    try:
        get_function(identifier)
        return True
    except CompileError:
        return False


def enter_scope(namespace: str) -> None:
    global _state
    _state = State(namespace, _state.global_, _state)


def exit_scope() -> None:
    global _state
    _state = _state.parent


def print_scope_info() -> None:
    global _state
    print(_state)
