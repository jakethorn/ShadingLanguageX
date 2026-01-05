#include <iostream>
#include "parse_args.h"
#include "compile.h"
#include "CompileError.h"

int main(const int argc, char* argv[])
{
    try
    {
        const mxslc::Args args = mxslc::parse_args(argc, argv);
        if (args.is_valid)
        {
            mxslc::compile(args.input_file);
        }
    }
    catch (mxslc::CompileError compile_error)
    {
        std::cerr << compile_error.what();
    }

    return 0;
}
