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
    void prepend_mxsl_stdlib(vector<Token>& tokens)
    {
        const fs::path stdlib_path = get_libraries_dir() / "stdlib.mxsl";
        vector<Token> stdlib_tokens = fscan(stdlib_path);
        tokens.insert(tokens.cbegin(), std::make_move_iterator(stdlib_tokens.begin()), std::make_move_iterator(stdlib_tokens.end()));
    }
}

void mxslc::compile(const fs::path& src_path, const fs::path& dst_path)
{
    vector<Token> tokens = fscan(src_path);
    prepend_mxsl_stdlib(tokens);
    const Runtime runtime;
    load_materialx_library(runtime, "1.39.4"s);
    const vector<StmtPtr> stmts = parse(runtime, std::move(tokens));
    runtime.enter_scope();
    for (const StmtPtr& stmt : stmts)
        stmt->execute();
    runtime.exit_scope();
    runtime.serializer().save(dst_path);
}

void mxslc::compile(const fs::path& src_path)
{
    fs::path dst_path = src_path;
    dst_path.replace_extension(".mtlx");

    compile(src_path, dst_path);
}
