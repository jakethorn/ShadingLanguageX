//
// Created by jaket on 02/01/2026.
//

#include <MaterialXFormat/XmlIo.h>

#include <utility>

#include "compile.h"

#include "common.h"
#include "CompileOptions.h"
#include "parse.h"
#include "scan.h"
#include "Token.h"
#include "debug/Debugger.h"
#include "runtime/Runtime.h"
#include "statements/Statement.h"
#include "utils/io_utils.h"
#include "errors/CompileError.h"
#include "preprocess/preprocess.h"
#include "runtime/utils/invoke.h"

namespace mxslc
{
    using debug::Debugger;

    namespace
    {
        fs::path get_destination_path(const fs::path& src_path, const CompileOptions& opts = {})
        {
            if (opts.output_file)
                return *opts.output_file;
            fs::path dst_path = src_path;
            dst_path.replace_extension(".mtlx");
            return dst_path;
        }

        fs::path get_stdlib_path(const CompileOptions& opts)
        {
            string searched_dirs;

            for (const fs::path& include_dir : opts.search_directories())
            {
                const fs::path lib_dir = include_dir / "libraries";
                searched_dirs += lib_dir.string() + "\n";

                const fs::path stdlib_path = lib_dir / "stdlib.mxsl";

                if (not fs::is_regular_file(stdlib_path))
                    continue;

                return stdlib_path;
            }

            throw CompileError{"ShadingLanguageX standard library could not be found.\nSearched directories:\n" + searched_dirs};
        }

        void parse_and_execute(vector<Token> tokens)
        {
            const vector<StmtPtr> stmts = parse(std::move(tokens));
            for (const StmtPtr& stmt : stmts)
                stmt->execute();
        }

        Runtime& compile(string source, CompileOptions opts, const optional<fs::path>& src_path)
        {
            opts.add_default_search_directories();

            const fs::path stdlib_path = get_stdlib_path(opts);
            vector<Token> stdlib_tokens = scan_file(stdlib_path);
            preprocess::preprocess(stdlib_tokens, opts, stdlib_path);

            vector<Token> tokens = scan_string(source, src_path);
            preprocess::preprocess(tokens, opts, src_path);

            Runtime& runtime = Runtime::create(opts);
            {
                runtime.enter_scope("mxsl_stdlib");
                parse_and_execute(std::move(stdlib_tokens));
                {
                    if (opts.debug_mode())
                        Debugger::create(std::move(source), src_path);

                    runtime.enter_scope("global");
                    parse_and_execute(std::move(tokens));
                    if (opts.has_entry_function())
                        runtime_utils::invoke_function(*opts.func_name, opts.entry_function_arguments());
                    runtime.exit_scope();

                    if (opts.debug_mode())
                        Debugger::close();
                }
                runtime.exit_scope();
            }

            runtime.finalise();
            return runtime;
        }
    }

    Runtime& compile(const fs::path& src_path)
    {
        return compile(io_utils::read_file(src_path), {}, src_path);
    }

    Runtime& compile(const fs::path& src_path, const CompileOptions& opts)
    {
        return compile(io_utils::read_file(src_path), opts, src_path);
    }

    Runtime& compile(const string& source)
    {
        return compile(source, {}, std::nullopt);
    }

    Runtime& compile(const string& source, const CompileOptions& opts)
    {
        return compile(source, opts, std::nullopt);
    }

    mx::DocumentPtr compile_to_document(const fs::path& src_path)
    {
        return compile(src_path).serializer().document();
    }

    mx::DocumentPtr compile_to_document(const fs::path& src_path, const CompileOptions& opts)
    {
        return compile(src_path, opts).serializer().document();
    }

    mx::DocumentPtr compile_to_document(const string& source)
    {
        return compile(source).serializer().document();
    }

    mx::DocumentPtr compile_to_document(const string& source, const CompileOptions& opts)
    {
        return compile(source, opts).serializer().document();
    }

    std::string compile_to_string(const fs::path& src_path)
    {
        return compile(src_path).serializer().xml();
    }

    std::string compile_to_string(const fs::path& src_path, const CompileOptions& opts)
    {
        return compile(src_path, opts).serializer().xml();
    }

    std::string compile_to_string(const string& source)
    {
        return compile(source).serializer().xml();
    }

    std::string compile_to_string(const string& source, const CompileOptions& opts)
    {
        return compile(source, opts).serializer().xml();
    }

    fs::path compile_to_file(const fs::path& src_path)
    {
        fs::path dst_path = get_destination_path(src_path);
        const string mtlx = compile_to_string(src_path);
        io_utils::save_file(dst_path, mtlx);
        return dst_path;
    }

    fs::path compile_to_file(const fs::path& src_path, const CompileOptions& opts)
    {
        fs::path dst_path = get_destination_path(src_path, opts);
        const string mtlx = compile_to_string(src_path, opts);
        io_utils::save_file(dst_path, mtlx);
        return dst_path;
    }

    fs::path compile_to_file(const fs::path& src_path, const fs::path& dst_path)
    {
        const string mtlx = compile_to_string(src_path);
        io_utils::save_file(dst_path, mtlx);
        return dst_path;
    }

    fs::path compile_to_file(const string& source, const CompileOptions& opts)
    {
        if (not opts.output_file)
            throw CompileError{"Please specify output file using CompileOptions.output_file."};
        const string mtlx = compile_to_string(source, opts);
        io_utils::save_file(*opts.output_file, mtlx);
        return *opts.output_file;
    }

    fs::path compile_to_file(const string& source, const fs::path& dst_path)
    {
        const string mtlx = compile_to_string(source);
        io_utils::save_file(dst_path, mtlx);
        return dst_path;
    }
}
