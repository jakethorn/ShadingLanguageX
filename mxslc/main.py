from argparse import ArgumentParser
from pathlib import Path

from mxslc import compile_file, Macro
from mxslc.CompileError import CompileError
from mxslc.scan import as_token


def _parse_main_args(args: list[str]) -> list[bool | int | float | str | Path]:
    parsed_args = []
    for arg in args:
        try:
            parsed_args.append(as_token(arg).value or arg)
        except CompileError:
            parsed_args.append(as_token('"' + arg + '"').value)
    return parsed_args


def _main(raw_args: list[str] = None):
    parser = ArgumentParser()
    parser.add_argument("mxsl_path", type=Path, help="Input path to mxsl file or containing folder")
    parser.add_argument("-o", "--output-path", type=Path, help="Output path of generated mtlx file or containing folder")
    parser.add_argument("-m", "--main-func", type=str, help="Name of main entry function into the program")
    parser.add_argument("-a", "--main-args", nargs="+", default=[], help="Arguments to be passed to the main function")
    parser.add_argument("-i", "--include-dirs", nargs="+", default=[], type=Path, help="Additional directories to search when including files")
    parser.add_argument("-d", "--define", dest="macros", nargs="+", action="append", default=[], type=str, help="Additional macro definitions")
    parser.add_argument("-v", "--validate", action="store_true", help="Validate the output MaterialX file")
    args = parser.parse_args(raw_args)

    try:
        compile_file(
            args.mxsl_path,
            args.output_path,
            main_func=args.main_func,
            main_args=_parse_main_args(args.main_args),
            add_include_dirs=args.include_dirs,
            add_macros=[Macro(*m) for m in args.macros],
            validate=args.validate
        )
    except Exception as e:
        print(e)


if __name__ == "__main__":
    _main()
