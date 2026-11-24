//
// Created by jaket on 14/11/2025.
//

#include <cassert>

#include "errors/CompileError.h"
#include "Token.h"
#include "utils/Stringable.h"

namespace mxslc
{
    CompileError::CompileError(string message) : diagnostics_{Diagnostic{SourceLocation{}, std::move(message)}}
    {

    }

    CompileError::CompileError(const Token& token, string message) : diagnostics_{Diagnostic{SourceLocation{token}, std::move(message)}}
    {

    }

    CompileError::CompileError(vector<Diagnostic> diagnostics) : diagnostics_{std::move(diagnostics)}
    {

    }

    const char* CompileError::what() const noexcept
    {
        what_ = Stringable::join(diagnostics_, "\n");
        return what_.c_str();
    }

    const string& CompileError::message() const
    {
        assert(diagnostics_.size() == 1);
        return diagnostics_.front().message;
    }

    void CompileError::set_debug_info(const Token& token)
    {
        for (Diagnostic& diagnostic : diagnostics_)
        {
            if (not diagnostic.location.is_known())
                diagnostic.location = SourceLocation{token};
        }
    }
}
