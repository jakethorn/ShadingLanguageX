//
// Created by jaket on 04/11/2026.
//

#include "reflection/reflect.h"

#include <algorithm>

#include "compile.h"
#include "TokenType.h"
#include "runtime/Runtime.h"

namespace mxslc::reflection
{
    namespace
    {
        CompileOptions get_reflect_options(const CompileOptions& opts = {})
        {
            CompileOptions reflect_opts = opts;
            reflect_opts.error_on_missing_globals = false;
            reflect_opts.error_on_unused_globals = false;
            reflect_opts.set_debug_mode(false);
            reflect_opts.enable_reflection();
            return reflect_opts;
        }
    }

    Reflection reflect()
    {
        return reflect(string{});
    }

    Reflection reflect(const CompileOptions& opts)
    {
        return reflect(string{}, opts);
    }

    Reflection reflect(const fs::path& src_path)
    {
        return compile(src_path, get_reflect_options()).reflection();
    }

    Reflection reflect(const fs::path& src_path, const CompileOptions& opts)
    {
        return compile(src_path, get_reflect_options(opts)).reflection();
    }

    Reflection reflect(const string& source)
    {
        return compile(source, get_reflect_options()).reflection();
    }

    Reflection reflect(const string& source, const CompileOptions& opts)
    {
        return compile(source, get_reflect_options(opts)).reflection();
    }

    vector<string> get_keywords()
    {
        vector<string> keywords;
        for (const TokenType keyword : TokenType::Keywords)
            keywords.push_back(keyword.to_string());
        std::sort(keywords.begin(), keywords.end());
        return keywords;
    }
}
