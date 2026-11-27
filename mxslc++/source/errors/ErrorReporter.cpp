//
// Created by jaket on 23/11/2026.
//

#include "errors/ErrorReporter.h"
#include "errors/CompileError.h"
#include "errors/FatalError.h"
#include "utils/container_utils.h"

namespace mxslc
{
    using container_utils::contains;

    ErrorReporter::ErrorReporter(const size_t max_errors) : max_errors_{max_errors}
    {

    }

    void ErrorReporter::report(const CompileError& e)
    {
        for (const Diagnostic& diagnostic : e.diagnostics())
            add(diagnostic);
    }

    void ErrorReporter::throw_if_errors() const
    {
        if (has_errors())
            throw CompileError{diagnostics_};
    }

    void ErrorReporter::add(const Diagnostic& diagnostic)
    {
        if (contains(diagnostics_, diagnostic))
            return;

        diagnostics_.push_back(diagnostic);

        if (is_at_limit())
        {
            diagnostics_.push_back(Diagnostic{SourceLocation{}, "Too many errors, stopping compilation"});
            throw FatalError{diagnostics_};
        }
    }

    bool ErrorReporter::is_at_limit() const
    {
        return max_errors_ != 0 and diagnostics_.size() >= max_errors_;
    }
}
