//
// Created by jaket on 03/01/2026.
//

#include "utils/parse_cli_args.h"

#include <iostream>
#include <fstream>
#include <functional>

#include "common.h"
#include "utils/parse_utils.h"
#include "utils/container_utils.h"
#include "utils/io_utils.h"
#include "utils/Span.h"
#include "errors/CompileError.h"
#include "runtime/interface.h"

namespace mxslc
{
    using std::ifstream;
    using std::function;

    using container_utils::contains;

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
actions:
  compile                        Compile the .mxsl file to .mtlx (default)
  decompile                      Decompile the .mtlx file to .mxsl

positional arguments:
  input-file                     Input path to .mxsl file

options:
  -h, --help                     Show this help message and exit
  -o, --output-file OUTPUT_FILE  Output path of .mtlx file
  -v, --version VERSION          Target MaterialX version (default: 1.39.5)
  -f, --func FUNC                Name of entry function into the program
  -a, --args ARGS*               Arguments to be passed to the entry function
  -g, --globals NAME VALUE*      Values to be assigned to `global` variables
  --missing-globals-ok           Allow `global` variables to be missing
  --unused-globals-ok            Allow `global` variables to be unused
  --no-reduce-graph              Always create graph nodes instead of evaluating logic at compile-time
)";

            std::cout << help_message;
        }

        Action to_action(const std::string& str)
        {
            if (str == "compile")
                return Action::Compile;
            else if (str == "decompile")
                return Action::Decompile;
            else
                return Action::Unknown;
        }
        void print_error(const string& message)
        {
            std::cerr << "Error: " << message << std::endl;
        }

        void print_warning(const string& message)
        {
            std::cerr << "Warning: " << message << std::endl;
        }

        vector<string> get_response_file_args(std::istream& file)
        {
            vector argv{"mxslc"s};

            bool in_quotes = false;
            bool in_comment = false;
            string arg;
            char c;
            while (file.get(c))
            {
                if (in_comment)
                {
                    in_comment = c != '\n';
                }
                else if (c == '"')
                {
                    arg += c;
                    in_quotes = not in_quotes;
                }
                else if (c == '#' and not in_quotes)
                {
                    if (not arg.empty())
                    {
                        argv.push_back(std::move(arg));
                        arg.clear();
                    }
                    in_comment = true;
                }
                else if (std::isspace(static_cast<unsigned char>(c)) and not in_quotes)
                {
                    if (not arg.empty())
                    {
                        argv.push_back(std::move(arg));
                        arg.clear();
                    }
                }
                else
                {
                    arg += c;
                }
            }

            if (not arg.empty())
                argv.push_back(std::move(arg));

            if (in_quotes)
            {
                print_error("Unterminated string in response file");
                return {};
            }

            return argv;
        }

        optional<Primitive> parse_literal(const string& str)
        {
            try
            {
                return parse_utils::parse_literal(str);
            }
            catch (const CompileError&)
            {
                return std::nullopt;
            }
        }

        void parse_action(Span<string>& argv, CommandLineArgs& clargs)
        {
            if (argv.front() == "compile" or argv.front() == "decompile")
                clargs.action = to_action(argv.pop_front());
        }

        void parse_input_file(Span<string>& argv, CommandLineArgs& clargs, const vector<fs::path>& search_dirs)
        {
            const fs::path path = argv.pop_front();
            for (const fs::path& dir : search_dirs)
            {
                fs::path abs_path = fs::absolute(dir / path).lexically_normal();
                if (fs::is_regular_file(abs_path))
                {
                    clargs.input_file = std::move(abs_path);
                    clargs.is_valid = true;
                    return;
                }
            }

            clargs.is_valid = false;
            print_error("Invalid input file path: " + path.string());
        }

        void parse_output_file(Span<string>& argv, CommandLineArgs& clargs)
        {
            if (argv.empty())
            {
                clargs.is_valid = false;
                print_error("Empty -o/--output-file option");
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
                print_error("Empty -v/--version option");
                return;
            }

            clargs.options.version = argv.pop_front();
        }

        void parse_function_name(Span<string>& argv, CommandLineArgs& clargs)
        {
            if (argv.empty())
            {
                clargs.is_valid = false;
                print_error("Empty -f/--func option");
                return;
            }

            clargs.options.func_name = argv.pop_front();
        }

        bool has_more_args(const Span<string>& argv)
        {
            if (argv.empty())
                return false;

            const string& arg = argv.front();

            if (arg.size() < 2)
                return true;

            const bool is_next_option = arg[0] == '-' and (std::isalpha(arg[1]) or arg[1] == '-');
            return not is_next_option;
        }

        void parse_function_args(Span<string>& argv, CommandLineArgs& clargs)
        {
            while (has_more_args(argv))
            {
                string front = argv.pop_front();
                optional<Primitive> value = parse_literal(front);
                if (not value)
                {
                    print_error("Invalid value in -a/--args option: " + front);
                    clargs.is_valid = false;
                    return;
                }

                clargs.options.add_entry_function_argument(create_variable(*value));
            }
        }

        void parse_globals(Span<string>& argv, CommandLineArgs& clargs)
        {
            while (has_more_args(argv))
            {
                string name = argv.pop_front();
                if (not has_more_args(argv))
                {
                    print_error("Missing value for -g/--globals option.");
                    clargs.is_valid = false;
                    return;
                }

                string front = argv.pop_front();
                optional<Primitive> value = parse_literal(front);
                if (not value)
                {
                    print_error("Invalid value in -g/--globals option: " + front);
                    clargs.is_valid = false;
                    return;
                }

                clargs.options.add_global(std::move(name), create_variable(*value));
            }
        }

        void parse_no_reduce_graph(Span<string>&, CommandLineArgs& clargs)
        {
            clargs.options.reduce_graph = false;
        }

        void parse_missing_globals_ok(Span<string>&, CommandLineArgs& clargs)
        {
            clargs.options.error_on_missing_globals = false;
        }

        void parse_unused_globals_ok(Span<string>&, CommandLineArgs& clargs)
        {
            clargs.options.error_on_unused_globals = false;
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
                {"-f", parse_function_name},
                {"--func", parse_function_name},
                {"-a", parse_function_args},
                {"--args", parse_function_args},
                {"-g", parse_globals},
                {"--globals", parse_globals},
                {"--missing-globals-ok", parse_missing_globals_ok},
                {"--unused-globals-ok", parse_unused_globals_ok},
                {"--no-reduce-graph", parse_no_reduce_graph}
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

    CommandLineArgs parse_cli_args(const int argc, char* argv[])
    {
        vector<string> args;
        args.reserve(argc);
        for (int i = 0; i < argc; ++i)
            args.emplace_back(argv[i]);
        return parse_cli_args(args);
    }

    CommandLineArgs parse_cli_args(const vector<string>& argv)
    {
        return parse_cli_args(argv, io_utils::get_default_search_directories());
    }

    CommandLineArgs parse_cli_args(const vector<string>& argv, const vector<fs::path>& search_dirs)
    {
        Span args{argv, 1};

        if (args.empty())
        {
            print_error("Input file path not specified");
            print_help();
            return CommandLineArgs::invalid();
        }

        if (args[0][0] == '@')
        {
            if (args.size() > 2)
                print_warning("Ignoring arguments after response file");
            return parse_cli_args(args[0].substr(1));
        }

        CommandLineArgs clargs;

        if (args.front() == "-h" or args.front() == "--help")
        {
            parse_help(args, clargs);
            if (args.empty())
                return clargs;
        }

        parse_action(args, clargs);
        parse_input_file(args, clargs, search_dirs);
        while (clargs.is_valid and not args.empty())
            parse_arg(args, clargs);

        return clargs;
    }

    CommandLineArgs parse_cli_args(const fs::path& response_path)
    {
        if (not fs::is_regular_file(response_path))
        {
            print_error("Invalid response file path: " + response_path.string());
            return CommandLineArgs::invalid();
        }

        ifstream file{response_path};
        if (not file.is_open())
        {
            print_error("Failed to open response file: " + response_path.string());
            return CommandLineArgs::invalid();
        }

        const vector argv = get_response_file_args(file);
        if (argv.empty())
        {
            print_error("Error occurred while parsing response file: " + response_path.string());
            return CommandLineArgs::invalid();
        }

        const vector<fs::path> search_dirs = io_utils::get_default_search_directories(response_path);
        return parse_cli_args(argv, search_dirs);
    }
}
