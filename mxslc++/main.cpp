#include <iostream>

#include "utils/parse_cli_args.h"
#include "compile.h"
#include "decompile/decompile.h"
#include "errors/CompileError.h"
#include "utils/Logger.h"

namespace
{
    void run_action(const mxslc::CommandLineArgs& args)
    {
        if (args.action == mxslc::Action::Compile)
        {
            const std::filesystem::path output_file = mxslc::compile_to_file(args.input_file, args.options);
            std::cout << "Material successfully compiled to " << output_file.string() << "\n";
        }

        if (args.action == mxslc::Action::Decompile)
        {
            const std::filesystem::path output_file = mxslc::decompile_to_file(args.input_file, args.options.output_file);
            std::cout << "Material successfully decompiled to " << output_file.string() << "\n";
        }
    }
}

int main(const int argc, char* argv[])
{
    try
    {
        if (const mxslc::CommandLineArgs args = mxslc::parse_cli_args(argc, argv); args.is_valid)
            run_action(args);
    }
    catch (const mxslc::CompileError& e)
    {
        mxslc::Logger::error(e);
        return 1;
    }
    catch (const std::exception& e)
    {
        mxslc::Logger::error(e.what());
        return 1;
    }

    return 0;
}
