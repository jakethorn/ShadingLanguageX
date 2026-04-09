//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_COMPILEERROR_H
#define FENNEC_COMPILEERROR_H

#include <stdexcept>
#include <string>

class Token;

namespace mxslc
{
    class CompileError final : public std::runtime_error
    {
    public:
        explicit CompileError(const std::string& message) : std::runtime_error{message} { }
        CompileError(const Token& debug_info, const std::string& message) : std::runtime_error{format(debug_info, message)} { }

    private:
        static std::string format(const Token& debug_info, const std::string& message);
    };
}

#endif //FENNEC_COMPILEERROR_H
