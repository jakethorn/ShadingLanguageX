//
// Created by jaket on 03/01/2026.
//

#include "parse_args.h"

#include <iostream>
#include <fstream>
#include <functional>

#include "utils/common.h"
#include "utils/template_utils.h"
#include "Span.h"

using std::ifstream;
using std::function;

CommandLineArgs CommandLineArgs::invalid()
{
    CommandLineArgs clargs;
    clargs.is_valid = false;
    return clargs;
}

namespace
{
    void print_help()
    {
        const string help_message =
R"(
positional arguments:
  input-file                        Input path to .mxsl file

options:
  -h, --help                       Show this help message and exit
  -o, --output-file OUTPUT_FILE    Output path of .mtlx file
  -v, --version VERSION            Target MaterialX version (default: 1.39.4)
  --no-reduce-graph                Always create graph nodes instead of evaluating logic at compile-time
  -m, --main-func MAIN_FUNC        Name of main entry function into the program
  -a, --main-args MAIN_ARGS        Arguments to be passed to the main function
  -i, --include-dirs INCLUDE_DIRS  Additional directories to search when including files
  -d, --define MACROS              Additional macro definitions
  --validate                       Validate the output MaterialX file
)";

        std::cout << help_message;
    }

    void print_error(const string& message)
    {
        std::cerr << "Error: " << message << std::endl;
    }

    void print_warning(const string& message)
    {
        std::cerr << "Warning: " << message << std::endl;
    }

    CommandLineArgs parse_response_file(const fs::path& response_file)
    {
        if (not fs::is_regular_file(response_file))
        {
            print_error("Invalid response file path: " + response_file.string());
            return CommandLineArgs::invalid();
        }

        ifstream file{response_file};
        if (not file.is_open())
        {
            print_error("Failed to open response file: " + response_file.string());
            return CommandLineArgs::invalid();
        }

        vector argv{"mxslc"s};
        string arg;
        while (file >> arg)
        {
            argv.push_back(std::move(arg));
        }

        return parse_args(argv);
    }

    void parse_input_file(Span<string>& argv, CommandLineArgs& clargs)
    {
        clargs.input_file = fs::absolute(argv.pop_front()).lexically_normal();
        clargs.is_valid = fs::is_regular_file(clargs.input_file);
        if (not clargs.is_valid)
            print_error("Invalid input file path: " + clargs.input_file.string());
    }

    void parse_output_file(Span<string>& argv, CommandLineArgs& clargs)
    {
        if (argv.empty())
        {
            clargs.is_valid = false;
            print_error("Empty -o/--output-file option"s);
            return;
        }

        clargs.options.output_file = fs::absolute(argv.pop_front());
    }

    void parse_help(Span<string>&, CommandLineArgs& clargs)
    {
        print_help();
        clargs.is_valid = false;
    }

    void parse_version(Span<string>& argv, CommandLineArgs& clargs)
    {
        if (argv.empty())
        {
            clargs.is_valid = false;
            print_error("Empty -v/--version option"s);
            return;
        }

        clargs.options.version = argv.pop_front();
    }

    void parse_no_reduce_graph(Span<string>&, CommandLineArgs& clargs)
    {
        clargs.options.reduce_graph = false;
    }

    void parse_arg(Span<string>& argv, CommandLineArgs& clargs)
    {
        const string& arg0 = argv.pop_front();

        unordered_map<string, function<void(Span<string>&, CommandLineArgs&)>> parse_map {
            {"-h", parse_help},
            {"--help", parse_help},
            {"-o", parse_output_file},
            {"--output-file", parse_output_file},
            {"-v", parse_version},
            {"--version", parse_version},
            {"--no-reduce-graph", parse_no_reduce_graph},
        };

        if (contains(parse_map, arg0))
        {
            parse_map[arg0](argv, clargs);
        }
        else
        {
            clargs.is_valid = false;
            print_error("Invalid option: " + arg0);
        }
    }
}

CommandLineArgs mxslc::parse_args(const int argc, char* argv[])
{
    vector<string> args;
    args.reserve(argc);
    for (int i = 0; i < argc; ++i)
        args.emplace_back(argv[i]);
    return parse_args(args);
}

CommandLineArgs mxslc::parse_args(const vector<string>& argv)
{
    Span args{argv, 1};

    if (args.empty())
    {
        print_error("Input file path not specified"s);
        print_help();
        return CommandLineArgs::invalid();
    }

    if (args[0][0] == '@')
    {
        if (args.size() > 2)
            print_warning("Ignoring arguments after response file"s);
        return parse_response_file(args[0].substr(1));
    }

    CommandLineArgs clargs;

    if (args.front() == "-h" or args.front() == "--help")
    {
        parse_help(args, clargs);
        if (args.empty())
            return clargs;
    }

    parse_input_file(args, clargs);
    while (clargs.is_valid and not args.empty())
        parse_arg(args, clargs);

    return clargs;
}
