from pathlib import Path


def get_data_path(filename: str = None) -> Path:
    path = Path(__file__).parent / "groundtruth"
    if filename:
        path = path / filename
    return path


def get_data(filename: str) -> str:
    return get_data_path(filename).read_text()


def write_data(filename: str, content: str) -> None:
    get_data_path(filename).write_text(content)
