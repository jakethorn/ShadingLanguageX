//
// Created by jaket on 27/06/2026.
//

#ifndef MXSLC_AMBIGUOUSFUNCTIONERROR_H
#define MXSLC_AMBIGUOUSFUNCTIONERROR_H

#include "utils/common.h"

class AmbiguousFunctionError final : public CompileError
{
public:
    explicit AmbiguousFunctionError(const string& message);
    explicit AmbiguousFunctionError(const string& func_name, const vector<FuncPtr>& funcs);
    explicit AmbiguousFunctionError(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors);

private:
    static string format(const string& func_name, const vector<FuncPtr>& funcs, const vector<string>& underlying_errors);
};

#endif //MXSLC_AMBIGUOUSFUNCTIONERROR_H
