from pathlib import Path


def handle_input_path(input_path: str | Path, extension=".mxsl") -> list[Path]:
    if input_path is None:
        raise TypeError(f"Path to {extension} file was empty.")
    if not isinstance(input_path, str | Path):
        raise TypeError(f"Path to {extension} file was an invalid type: '{type(input_path)}'.")
    input_path = Path(input_path).resolve()
    if not input_path.exists():
        raise FileNotFoundError(f"No such file or directory: '{input_path}'.")
    if input_path.is_file():
        return [input_path]
    if input_path.is_dir():
        input_filepaths = list(input_path.glob(f"*{extension}"))
        if len(input_filepaths) == 0:
            raise FileNotFoundError(f"No {extension} files found in directory: '{input_path}'.")
        return input_filepaths
    raise ValueError(f"Invalid input path: '{input_path}'.")


def handle_output_path(output_path: str | Path | None, input_filepath: Path, extension=".mtlx") -> Path:
    if output_path is None:
        return input_filepath.with_suffix(extension)
    if not isinstance(output_path, str | Path):
        raise TypeError(f"Path to {extension} file was an invalid type: '{type(output_path)}'.")
    output_path = Path(output_path).resolve()
    if output_path.is_file():
        return output_path
    if output_path.is_dir():
        return output_path / (input_filepath.stem + extension)
    if output_path.suffix != extension:
        output_path /= (input_filepath.stem + extension)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    return output_path


def pkg_path(relative_path: str) -> Path:
    return Path(__file__).parent / relative_path
