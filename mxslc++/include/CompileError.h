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
        explicit CompileError(const std::string& message) : std::runtime_error{message}, has_debug_info_{false} { }
        CompileError(const Token& debug_info, const std::string& message) : std::runtime_error{format(debug_info, message)}, has_debug_info_{true} { }
        CompileError(const Token& debug_info, const CompileError& error) : std::runtime_error{format(debug_info, error)}, has_debug_info_{true} { }

        bool has_debug_info() const { return has_debug_info_; }

    private:
        static std::string format(const Token& debug_info, const std::string& message);
        static std::string format(const Token& debug_info, const CompileError& error);

        bool has_debug_info_;
    };
}

#endif //FENNEC_COMPILEERROR_H
