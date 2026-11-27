//
// Created by jaket on 23/11/2026.
//

#ifndef MXSLC_ERRORREPORTER_H
#define MXSLC_ERRORREPORTER_H

#include "common.h"
#include "errors/Diagnostic.h"

namespace mxslc
{
    class CompileError;

    class ErrorReporter
    {
    public:
        explicit ErrorReporter(size_t max_errors);

        void report(const CompileError& e);
        void throw_if_errors() const;

        bool has_errors() const { return not diagnostics_.empty(); }
        const vector<Diagnostic>& diagnostics() const { return diagnostics_; }

    private:
        void add(const Diagnostic& diagnostic);
        bool is_at_limit() const;

        size_t max_errors_;
        vector<Diagnostic> diagnostics_;
    };
}

#endif //MXSLC_ERRORREPORTER_H
