from __future__ import annotations

from pathlib import Path
from typing import Sequence

import MaterialX as mx

from .InteractiveExpression import InteractiveExpression
from .InteractiveNode import InteractiveNode
from .. import state, mx_utils
from ..Argument import Argument
from ..CompileError import CompileError


class ShaderInterface:
    def __getattr__(self, name: str) -> InteractiveNode | InteractiveFunction:
        return self[name]

    def __setattr__(self, name: str, value: mx_utils.Value) -> None:
        self[name] = value

    def __contains__(self, name: str) -> bool:
        return state.is_node(name) or state.is_function(name)

    def __getitem__(self, name: str) -> InteractiveNode | InteractiveFunction:
        if state.is_node(name):
            return InteractiveNode(state.get_node(name))
        if state.is_function(name):
            return InteractiveFunction(name)
        raise CompileError(f"No variable or function named '{name}' found.")

    def __setitem__(self, name: str, value: mx_utils.Value) -> None:
        # TODO type checking
        if state.is_node(name):
            state.set_node(name, _to_mtlx_node(value))
        raise CompileError(f"No variable named '{name}' found.")

    def __len__(self) -> int:
        # TODO this. no variables + no functions
        raise NotImplementedError

    def __str__(self) -> str:
        # TODO this. list all variables and functions in the global scope
        raise NotImplementedError()


class InteractiveFunction:
    def __init__(self, name: str):
        self.__function = state.get_function(name)

    def __call__(self, *args: mx_utils.Value | InteractiveNode) -> InteractiveNode:
        node = self.__function.invoke(_to_arg_list(args))
        return InteractiveNode(node)

    @property
    def file(self) -> Path:
        return self.__function.file

    @property
    def line(self) -> int:
        return self.__function.line


def _to_mtlx_node(value: mx_utils.Value) -> mx_utils.Node:
    if isinstance(value, mx.Node):
        return mx_utils.Node(value)
    else:
        return mx_utils.constant(value)


def _to_arg_list(args: Sequence[mx_utils.Value | InteractiveNode]) -> list[Argument]:
    arg_list = []
    for i, arg in enumerate(args):
        if isinstance(arg, InteractiveNode):
            arg = arg.node
        expr_arg = Argument(InteractiveExpression(arg), i)
        expr_arg.init()
        arg_list.append(expr_arg)
    return arg_list
