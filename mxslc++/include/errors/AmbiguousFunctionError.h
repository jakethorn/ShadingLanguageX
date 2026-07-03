//
// Created by jaket on 27/06/2026.
//

#ifndef MXSLC_AMBIGUOUSFUNCTIONERROR_H
#define MXSLC_AMBIGUOUSFUNCTIONERROR_H

#include "CompileError.h"
#include "common.h"

namespace mxslc
{
    class AmbiguousFunctionError final : public CompileError
    {
    public:
        explicit AmbiguousFunctionError(const string& message);
        AmbiguousFunctionError(const string& func_name, const vector<FuncPtr>& funcs);
        AmbiguousFunctionError(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors);

    private:
        static string format(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors);
    };
}

#endif //MXSLC_AMBIGUOUSFUNCTIONERROR_H
