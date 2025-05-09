from __future__ import annotations

from pathlib import Path
from typing import Sequence

import MaterialX as mx

from .. import state, mtlx
from ..Argument import Argument
from ..CompileError import CompileError
from ..Expressions import NodeExpression, ConstantExpression
from ..Keyword import DataType
from ..Parameter import Parameter


class ShaderInterface:
    def __getattr__(self, name: str) -> mx.Node | Function:
        return self[name]

    def __setattr__(self, name: str, value: mx.Node | mtlx.Constant) -> None:
        self[name] = value

    def __contains__(self, name: str) -> bool:
        return state.is_node(name) or state.is_function(name)

    def __getitem__(self, name: str) -> mx.Node | Function:
        if state.is_node(name):
            return mtlx.get_source(state.get_node(name))
        if state.is_function(name):
            return Function(name)
        raise CompileError(-1, f"No variable or function: '{name}'.")

    def __setitem__(self, name: str, value: mx.Node | mtlx.Constant) -> None:
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

    def __call__(self, *args: mx.Node | mtlx.Constant) -> mx.Node:
        node = self.__function.invoke(_to_arg_list(args))
        return mtlx.get_source(node)

    @property
    def name(self) -> str:
        return self.__function.name

    @property
    def return_type(self) -> DataType:
        return self.__function.data_type

    @property
    def parameters(self) -> list[Parameter]:
        # TODO change type hint from list[parameter] to ParameterList
        return self.__function.params

    @property
    def file(self) -> Path:
        return self.__function.file

    @property
    def line(self) -> int:
        return self.__function.line


def _to_mtlx_node(value: mx.Node | mtlx.Constant) -> mtlx.Node:
    if isinstance(value, mx.Node):
        return mtlx.Node(value)
    return mtlx.constant(value)


def _to_arg_list(args: Sequence[mx.Node | mtlx.Constant]) -> list[Argument]:
    return [
        Argument(NodeExpression(mtlx.Node(a)) if isinstance(a, mx.Node) else ConstantExpression(a))
        for a
        in args
    ]
