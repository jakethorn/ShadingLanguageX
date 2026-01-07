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

void mxslc::compile(const fs::path& src_path, const fs::path& dst_path)
{
    vector<Token> tokens = fscan(src_path);
    const Runtime runtime;
    load_materialx_library(runtime, "1.39.4"s);
    const vector<StmtPtr> stmts = parse(runtime, std::move(tokens));
    for (const StmtPtr& stmt : stmts)
        stmt->execute();
    runtime.serializer().save(dst_path);
}

void mxslc::compile(const fs::path& src_path)
{
    fs::path dst_path = src_path;
    dst_path.replace_extension(".mtlx"s);

    compile(src_path, dst_path);
}
