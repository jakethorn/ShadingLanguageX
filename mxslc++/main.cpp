#include "parse_args.h"
#include "compile.h"

int main(const int argc, char* argv[])
{
    if (const mxslc::Args args = mxslc::parse_args(argc, argv); args.is_valid)
    {
        mxslc::compile(args.input_file);
    }

    return 0;
}
