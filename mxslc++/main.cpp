#include "parse_args.h"
#include "compile.h"

int main(const int argc, char* argv[])
{
    if (const mxslc::CommandLineArgs args = mxslc::parse_args(argc, argv); args.is_valid)
    {
        mxslc::compile_to_file(args.input_file, args.options);
    }

    return 0;
}
