//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_COMPILEERROR_H
#define FENNEC_COMPILEERROR_H

#include <exception>

#include "common.h"
#include "errors/Diagnostic.h"

namespace mxslc
{
    class Token;

    class CompileError : public std::exception
    {
    public:
        explicit CompileError(string message);
        CompileError(const Token& token, string message);
        explicit CompileError(vector<Diagnostic> diagnostics);

        const char* what() const noexcept override;

        const vector<Diagnostic>& diagnostics() const { return diagnostics_; }
        const string& message() const;

        void set_debug_info(const Token& token);

    private:
        vector<Diagnostic> diagnostics_;
        mutable string what_;
    };
}

#endif //FENNEC_COMPILEERROR_H
