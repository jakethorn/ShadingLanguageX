from __future__ import annotations

from . import mtlx
from .CompileError import CompileError
from .Statements import FunctionDeclaration, ForLoop
from .Token import Token
from .scan import as_token


class _State:
    def __init__(self, namespace: str, global_: _State = None, parent: _State = None):
        self.namespace = namespace
        self.global_ = global_ or self
        self.parent = parent
        self.nodes: dict[str, mtlx.Node] = {}
        self.functions: dict[str, FunctionDeclaration] = {}

    def add_node(self, identifier: Token, node: mtlx.Node) -> None:
        name = identifier.lexeme
        if name in self.nodes:
            raise CompileError(identifier.line, f"Variable name '{name}' already exists.")
        assert node not in self.nodes.values()
        self.nodes[name] = node
        node.name = self.get_full_name(name)

    def get_node(self, identifier: Token) -> mtlx.Node:
        name = identifier.lexeme
        if name in self.nodes:
            return self.nodes[name]
        if self.namespace == ForLoop.NAMESPACE:
            try:
                return self.parent.get_node(identifier)
            except CompileError:
                ...
        if name in self.global_.nodes:
            return self.global_.nodes[name]
        raise CompileError(identifier.line, f"Variable name '{name}' does not exist.")

    def set_node(self, identifier: Token, node: mtlx.Node) -> None:
        name = identifier.lexeme
        assert node not in self.nodes.values()
        if name in self.nodes:
            self.nodes[name] = node
            node.name = self.get_full_name(name)
            return
        if self.namespace == ForLoop.NAMESPACE:
            try:
                self.parent.set_node(identifier, node)
                return
            except CompileError:
                ...
        # If I wanted to let developers set global variables I would do it here
        raise CompileError(identifier.line, f"Variable name '{name}' does not exist.")

    def clear(self) -> None:
        self.nodes.clear()
        self.functions.clear()

    def get_full_name(self, name: str) -> str:
        return f"{self.namespace}__{name}"

    def add_function(self, func: FunctionDeclaration) -> None:
        if func.name in self.functions:
            raise CompileError(func.line, f"Function name '{func.name}' already exists.")
        assert func not in self.functions.values()
        self.functions[func.name] = func

    def get_function(self, identifier: Token) -> FunctionDeclaration:
        name = identifier.lexeme
        if name in self.functions:
            return self.functions[name]
        if name in self.global_.functions:
            return self.global_.functions[name]
        raise CompileError(identifier.line, f"Function name '{name}' does not exist.")

    def __str__(self) -> str:
        output = ""
        output += "self: " + self.namespace + "\n"
        output += "parent: " + (self.parent.namespace if self.parent else "None") + "\n"
        output += "global: " + self.global_.namespace + "\n"
        output += "----------------" + "\n"
        for name, node in self.nodes.items():
            output += f"{name}: {node.data_type} {node.name}\n"
        return output


_state = _State("global")


def add_node(identifier: Token, node: mtlx.Node) -> None:
    _state.add_node(identifier, node)


def get_node(identifier: str | Token) -> mtlx.Node:
    return _state.get_node(as_token(identifier))


def set_node(identifier: str | Token, node: mtlx.Node) -> None:
    _state.set_node(as_token(identifier), node)


def is_node(identifier: str) -> bool:
    try:
        get_node(identifier)
        return True
    except CompileError:
        return False


def clear() -> None:
    global _state
    while _state.namespace != "global":
        _state = _state.parent
    _state.clear()


def add_function(func: FunctionDeclaration) -> None:
    _state.add_function(func)


def get_function(identifier: str | Token) -> FunctionDeclaration:
    return _state.get_function(as_token(identifier))


def is_function(identifier: str) -> bool:
    try:
        get_function(identifier)
        return True
    except CompileError:
        return False


def enter_scope(namespace: str) -> None:
    global _state
    _state = _State(namespace, _state.global_, _state)


def exit_scope() -> None:
    global _state
    _state = _state.parent


def print_scope_info() -> None:
    global _state
    print(_state)
