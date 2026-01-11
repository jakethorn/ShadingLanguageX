//
// Created by jaket on 02/01/2026.
//

#include "compile.h"

#include "utils/common.h"
#include "parse.h"
#include "scan.h"
#include "Token.h"
#include "mtlx/load_mtlx.h"
#include "statements/Statement.h"
#include "utils/io_utils.h"

namespace
{
    void compile(const Runtime& runtime, const fs::path& src_path)
    {
        vector<Token> tokens = fscan(src_path);
        const vector<StmtPtr> stmts = parse(runtime, std::move(tokens));
        for (const StmtPtr& stmt : stmts)
            stmt->execute();
    }

    void compile_mxsl_stdlib(const Runtime& runtime)
    {
        const fs::path stdlib_path = get_libraries_dir() / "stdlib.mxsl";
        compile(runtime, stdlib_path);
    }
}

void mxslc::compile_to_file(const fs::path& src_path, const fs::path& dst_path)
{
    const Runtime runtime;
    load_materialx_library(runtime, "1.39.4"s);
    {
        runtime.enter_scope();
        compile_mxsl_stdlib(runtime);
        {
            runtime.enter_scope();
            compile(runtime, src_path);
            runtime.exit_scope();
        }
        runtime.exit_scope();
    }
    runtime.serializer().save(dst_path);
}

fs::path mxslc::compile_to_file(const fs::path &src_path)
{
    fs::path dst_path = src_path;
    dst_path.replace_extension(".mtlx");

    compile_to_file(src_path, dst_path);

    return dst_path;
}
