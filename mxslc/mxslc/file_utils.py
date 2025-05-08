from pathlib import Path


def handle_mxsl_path(mxsl_path: str | Path) -> list[Path]:
    if mxsl_path is None:
        raise TypeError("Path to .mxsl file was empty.")
    if not isinstance(mxsl_path, str | Path):
        raise TypeError(f"Path to .mxsl file was an invalid type: '{type(mxsl_path)}'.")
    mxsl_path = Path(mxsl_path).resolve()
    if not mxsl_path.exists():
        raise FileNotFoundError(f"No such file or directory: '{mxsl_path}'.")
    if mxsl_path.is_file():
        return [mxsl_path]
    if mxsl_path.is_dir():
        return list(mxsl_path.glob("*.mxsl"))
    raise ValueError("Bad mxsl_path.")


def handle_mtlx_path(mtlx_path: str | Path | None, mxsl_file: Path) -> Path:
    if mtlx_path is None:
        return mxsl_file.with_suffix(".mtlx")
    if not isinstance(mtlx_path, str | Path):
        raise TypeError(f"Path to .mtlx file was an invalid type: '{type(mtlx_path)}'.")
    mtlx_path = Path(mtlx_path).resolve()
    if mtlx_path.is_file():
        return mtlx_path
    if mtlx_path.is_dir():
        return mtlx_path / (mxsl_file.stem + ".mtlx")
    if mtlx_path.suffix != ".mtlx":
        mtlx_path /= (mxsl_file.stem + ".mtlx")
    mtlx_path.parent.mkdir(parents=True, exist_ok=True)
    return mtlx_path
