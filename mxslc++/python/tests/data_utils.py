from pathlib import Path


def get_data_path(filename: str = None) -> Path:
    __tracebackhide__ = True

    path = Path(__file__).parent / "groundtruth"
    if filename:
        path = path / filename
    return path


def get_data(filename: str) -> str:
    __tracebackhide__ = True

    return get_data_path(filename).read_text()


def assert_matches_groundtruth(actual: str, filename: str, override: bool = False) -> None:
    __tracebackhide__ = True

    if False or override:
        get_data_path(filename).write_text(actual)
    assert actual == get_data(filename)
