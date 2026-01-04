//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_COMPILEERROR_H
#define FENNEC_COMPILEERROR_H

#include <stdexcept>
#include "utils/common.h"

using std::runtime_error;

class Token;

class CompileError final : public runtime_error
{
public:
    explicit CompileError(const string& message) : runtime_error{message} { }
    CompileError(const Token& debug_info, const string& message) : runtime_error{format(debug_info, message)} { }

private:
    static string format(const Token& debug_info, const string& message);
};

#endif //FENNEC_COMPILEERROR_H
