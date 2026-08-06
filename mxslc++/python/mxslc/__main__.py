import argparse
import sys

from . import __version__, compile_file_to_file, decompile_file_to_file


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="mxslcxx", description="mxslc++ Python CLI")
    parser.add_argument("--version", action="version", version=f"%(prog)s {__version__}")

    subparsers = parser.add_subparsers(dest="command", required=True)

    compile_parser = subparsers.add_parser("compile", help="Compile .mxsl to .mtlx")
    compile_parser.add_argument("input", help="Source .mxsl path")
    compile_parser.add_argument("-o", "--output", help="Destination .mtlx path")

    decompile_parser = subparsers.add_parser("decompile", help="Decompile .mtlx to .mxsl")
    decompile_parser.add_argument("input", help="Source .mtlx path")
    decompile_parser.add_argument("-o", "--output", help="Destination .mxsl path")

    # Add version subcommand for convenience
    version_parser = subparsers.add_parser("version", help="Show version information")  
    version_parser.set_defaults(func=lambda args: print(f"mxslcxx {__version__}"))

    return parser


def main(argv: list[str] | None = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)

    try:
        if hasattr(args, "func"):
            args.func(args)
            return 0
        
        if args.command == "compile":
            result = compile_file_to_file(args.input, args.output) if args.output else compile_file_to_file(args.input)
            print(result)
            return 0

        if args.command == "decompile":
            result = decompile_file_to_file(args.input, args.output) if args.output else decompile_file_to_file(args.input)
            print(result)
            return 0
    except Exception as exc:
        print(str(exc), file=sys.stderr)
        return 1

    parser.print_help()
    return 2


if __name__ == "__main__":
    raise SystemExit(main())
