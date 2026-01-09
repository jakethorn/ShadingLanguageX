#include "parse_args.h"
#include "compile.h"

int main(const int argc, char* argv[])
{
    if (const mxslc::CompileOptions opts = mxslc::parse_args(argc, argv); opts.is_valid)
    {
        mxslc::compile(opts.input_file);
    }

    return 0;
}
