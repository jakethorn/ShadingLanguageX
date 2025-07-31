import sys
import argparse
from pathlib import Path

try:
    from mxslc.Decompiler.decompile import Decompiler
except ImportError as e:
    print(f"Error: Could not import decompiler. Error: {e}")
    sys.exit(1)

try:
    from mxslc.Decompiler.decompile import decompile_string
except ImportError as e:
    print(f"Error: Could not import decompile_string function. Error: {e}")


def decompile_string_local(mtlx_content: str) -> str:    
    """
    Decompile MaterialX in XML format to SLX string.
    """
    decompiler = Decompiler(mtlx_content)
    return decompiler.decompile()

def main():
    parser = argparse.ArgumentParser(description="Convert MaterialX files to SLX format.")
    parser.add_argument(dest='input_file', help="Path to the input MaterialX file (.mtlx)")
    parser.add_argument("-o", "--output_file", dest="output_file", default="", type=str, help="Path to the output SLX file (.slx). If not provided, output will be printed to stdout.")

    args = parser.parse_args()
    input_file = Path(args.input_file)
    if not input_file.exists():
        print(f"Input file does not exist: {input_file}")
        sys.exit(1)
    
    mtlx_content = ""
    with open(input_file, 'r', encoding='utf-8') as f:
        mtlx_content = f.read()

    try:
        if decompile_string:
            mxsl_output = decompile_string(mtlx_content)
        else:
            mxsl_output = decompile_string_local(mtlx_content)
    except Exception as e:
        print(f"Error during decompilation: {e}")
        sys.exit(1)

    if not mxsl_output:
        print("No output produced from decompilation.")
        sys.exit(1)
    
    output_file = args.output_file
    if len(output_file) == 0:
        print(mxsl_output)
    else:
        output_file = Path(output_file)
        with open(output_file, "w") as f:
            f.write(mxsl_output)
        print(f"Output written to: {output_file}")
    
if __name__ == "__main__":
    main()
