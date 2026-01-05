//
// Created by jaket on 02/11/2025.
//

#ifndef MXSLC_COMPILEERROR_H
#define MXSLC_COMPILEERROR_H

#include <stdexcept>

namespace
{
    class Token;
}

namespace mxslc
{
    class CompileError final : public std::runtime_error
    {
    public:
        explicit CompileError(const string& message) : std::runtime_error{ message } {}
        CompileError(const Token& debug_info, const string& message) : std::runtime_error{ format(debug_info, message) } {}

    private:
        static string format(const Token& debug_info, const string& message);
    };
}

#endif //MXSLC_COMPILEERROR_H
