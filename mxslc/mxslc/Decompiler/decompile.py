from pathlib import Path

import MaterialX as mx

from .Argument import Argument


def decompile_file(mtlx_path: str | Path, mxsl_path: str | Path = None) -> None:
    # check arg validity
    err_msg = f"Invalid Materialx filepath: {mtlx_path}"
    assert mtlx_path is not None, err_msg
    assert isinstance(mtlx_path, str | Path), err_msg
    mtlx_path = Path(mtlx_path)
    assert mtlx_path.is_file(), err_msg
    assert mxsl_path is None or isinstance(mxsl_path, str | Path), "Invalid SLX path"
    if mxsl_path is None:
        mxsl_path = mtlx_path.with_suffix('.mxsl')
    elif Path(mxsl_path).is_dir():
        mxsl_path = Path(mxsl_path) / mtlx_path.with_suffix('.mxsl').name

    # decompile
    document: mx.Document = _load_mtlx_file(mtlx_path)
    nodes: list[mx.Node] = document.getNodes()
    for node in nodes:
        var_type = node.getType()
        var_name = node.getName()
        func_name = node.getCategory()
        args: list[Argument] = []
        inputs: list[mx.Input] = node.getInputs()
        for input in inputs:
            name = input.getName()
            node: mx.Node = input.getConnectedNode()
            if node:
                value = node.getName()
            else:
                value = input.getValueString()
            args.append(Argument(name, value))
        stmt = f"{var_type} {var_name} = {func_name}({', '.join([str(a) for a in args])});"
        print(stmt)


def _load_mtlx_file(mtlx_path: str | Path) -> mx.Document:
    doc: mx.Document = mx.createDocument()
    mx.readFromXmlFile(doc, str(mtlx_path))
    return doc
