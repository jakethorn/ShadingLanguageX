from __future__ import annotations

from pathlib import Path
from typing import Sequence

import MaterialX as mx

from .InteractiveExpression import InteractiveExpression
from .InteractiveNode import InteractiveNode
from .. import state, mtlx
from ..Argument import Argument
from ..CompileError import CompileError
from ..Expressions import StandardLibraryCall
from ..StandardLibrary import StandardLibrary
from ..Token import Token
from ..token_types import IDENTIFIER


class ShaderInterface:
    def __getattr__(self, name: str) -> InteractiveNode | Function:
        return self[name]

    def __setattr__(self, name: str, value: mtlx.Value) -> None:
        self[name] = value

    def __contains__(self, name: str) -> bool:
        return state.is_node(name) or state.is_function(name)

    def __getitem__(self, name: str) -> InteractiveNode | Function:
        if state.is_node(name):
            return InteractiveNode(state.get_node(name))
        if state.is_function(name):
            return Function(name)
        if name in StandardLibrary:
            return StandardLibraryFunction(name)
        raise CompileError(-1, f"No variable or function: '{name}'.")

    def __setitem__(self, name: str, value: mtlx.Value) -> None:
        # TODO type checking
        if state.is_node(name):
            state.set_node(name, _to_mtlx_node(value))
        raise CompileError(-1, f"No variable: '{name}'.")

    def __len__(self) -> int:
        # TODO this. no variables + no functions
        raise NotImplementedError

    def __str__(self) -> str:
        # TODO this. list all variables and functions in the global scope
        raise NotImplementedError()


class Function:
    def __init__(self, name: str):
        self.__function = state.get_function(name)

    def __call__(self, *args: mtlx.Value | InteractiveNode) -> InteractiveNode:
        node = self.__function.invoke(_to_arg_list(args))
        return InteractiveNode(node)

    @property
    def file(self) -> Path:
        return self.__function.file

    @property
    def line(self) -> int:
        return self.__function.line


class StandardLibraryFunction:
    def __init__(self, name: str):
        self.__name = name

    def __call__(self, *args: mtlx.Value | InteractiveNode) -> InteractiveNode:
        call = StandardLibraryCall(Token(IDENTIFIER, self.__name), _to_arg_list(args))
        return InteractiveNode(call.evaluate())


def _to_mtlx_node(value: mtlx.Value) -> mtlx.Node:
    if isinstance(value, mx.Node):
        return mtlx.Node(value)
    return mtlx.constant(value)


def _to_arg_list(args: Sequence[mtlx.Value | InteractiveNode]) -> list[Argument]:
    arg_list = []
    for arg in args:
        if isinstance(arg, InteractiveNode):
            arg = arg.node
        arg_list.append(Argument(InteractiveExpression(arg)))
    return arg_list
