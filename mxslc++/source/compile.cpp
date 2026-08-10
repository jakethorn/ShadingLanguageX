//
// Created by jaket on 02/01/2026.
//

#include <MaterialXFormat/XmlIo.h>

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
        void compile_tokens(vector<Token> tokens)
        {
            const vector<StmtPtr> stmts = parse(std::move(tokens));
            for (const StmtPtr& stmt : stmts)
                stmt->execute();
        }

        void compile_file(const fs::path& src_path)
        {
            vector<Token> tokens = scan_file(src_path);
            compile_tokens(std::move(tokens));
        }

        void compile_string(const string& source)
        {
            vector<Token> tokens = scan_string(source);
            compile_tokens(std::move(tokens));
        }

        void compile_mxsl_stdlib()
        {
            string searched_dirs;

            for (const fs::path& include_dir : Runtime::get().include_directories())
            {
                const fs::path lib_dir = include_dir / "libraries";
                searched_dirs += lib_dir.string() + "\n";

                const fs::path stdlib_path = lib_dir / "stdlib.mxsl";

                if (not fs::is_regular_file(stdlib_path))
                    continue;

                compile_file(stdlib_path);
                return;
            }

            throw CompileError{"ShadingLanguageX standard library could not be found.\nSearched directories:\n" + searched_dirs};
        }

        mx::DocumentPtr compile_to_document(string source, CompileOptions opts, const optional<fs::path>& src_path)
        {
            vector<Token> tokens = scan_string(source, src_path);

            opts.add_default_search_directories();
            preprocess::preprocess(tokens, opts, src_path);

            Runtime& runtime = Runtime::create(opts);
            {
                runtime.enter_scope("mxsl_stdlib");
                compile_mxsl_stdlib();
                {
                    if (opts.debug_mode())
                        Debugger::create(std::move(source));

                    runtime.enter_scope("global");
                    compile_tokens(std::move(tokens));
                    if (opts.has_entry_function())
                        runtime_utils::invoke_function(*opts.func_name, opts.entry_function_arguments());
                    runtime.exit_scope();
                }
                runtime.exit_scope();
            }

            return runtime.serializer().document();
        }

        fs::path get_destination_path(const fs::path& src_path, const CompileOptions& opts)
        {
            if (opts.output_file)
                return *opts.output_file;
            fs::path dst_path = src_path;
            dst_path.replace_extension(".mtlx");
            return dst_path;
        }
    }

    mx::DocumentPtr compile_to_document(const fs::path& src_path)
    {
        return compile_to_document(src_path, CompileOptions{});
    }

    mx::DocumentPtr compile_to_document(const fs::path& src_path, const CompileOptions& opts)
    {
        return compile_to_document(io_utils::read_file(src_path), opts, src_path);
    }

    std::string compile_to_string(const fs::path& src_path)
    {
        return compile_to_string(src_path, CompileOptions{});
    }

    std::string compile_to_string(const fs::path& src_path, const CompileOptions& opts)
    {
        const mx::DocumentPtr doc = compile_to_document(src_path, opts);
        return mx::writeToXmlString(doc);
    }

    fs::path compile_to_file(const fs::path& src_path)
    {
        return compile_to_file(src_path, CompileOptions{});
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
        CompileOptions opts;
        opts.output_file = dst_path;

        return compile_to_file(src_path, opts);
    }

    mx::DocumentPtr compile_to_document(const string& source)
    {
        return compile_to_document(source, CompileOptions{});
    }

    mx::DocumentPtr compile_to_document(const string& source, const CompileOptions& opts)
    {
        return compile_to_document(source, opts, std::nullopt);
    }

    std::string compile_to_string(const string& source)
    {
        return compile_to_string(source, CompileOptions{});
    }

    std::string compile_to_string(const string& source, const CompileOptions& opts)
    {
        const mx::DocumentPtr doc = compile_to_document(source, opts);
        return mx::writeToXmlString(doc);
    }

    fs::path compile_to_file(const string& source, const CompileOptions& opts)
    {
        if (not opts.output_file)
            throw CompileError{"Output file not specified"s};
        const string mtlx = compile_to_string(source, opts);
        io_utils::save_file(*opts.output_file, mtlx);
        return *opts.output_file;
    }

    fs::path compile_to_file(const string& source, const fs::path& dst_path)
    {
        CompileOptions opts;
        opts.output_file = dst_path;

        return compile_to_file(source, opts);
    }
}
