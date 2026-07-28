#include <iostream>

#include "utils/parse_cli_args.h"
#include "compile.h"
#include "decompile/decompile.h"

int main(const int argc, char* argv[])
{
    if (const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argc, argv); args.is_valid)
    {
        if (args.action == mxslc::Action::Compile)
        {
            const std::filesystem::path output_file = mxslc::compile_to_file(args.input_file, args.options);
            std::cout << "\nMaterial successfully compiled to " << output_file.string();
        }

        if (args.action == mxslc::Action::Decompile)
        {
            const std::filesystem::path output_file = mxslc::decompile_to_file(args.input_file, args.options.output_file);
            std::cout << "\nMaterial successfully decompiled to " << output_file.string();
        }
    }

    return 0;
}
