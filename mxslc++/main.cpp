#include <iostream>

#include "parse_args.h"
#include "compile.h"

int main(const int argc, char* argv[])
{
    if (const mxslc::CommandLineArgs args = mxslc::parse_args(argc, argv); args.is_valid)
    {
        const std::filesystem::path output_file = mxslc::compile_to_file(args.input_file, args.options);
        std::cout << "Material successfully compiled to " << output_file.string() << std::endl;
    }

    return 0;
}
