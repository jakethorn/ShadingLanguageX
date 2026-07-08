//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_COMPILEERROR_H
#define FENNEC_COMPILEERROR_H

#include <exception>

#include "common.h"

namespace mxslc
{
    class Token;

    class CompileError : public std::exception
    {
    public:
        explicit CompileError(string message) : message_{std::move(message)}, has_debug_info_{false} { }
        CompileError(const Token& debug_info, const string& message) : message_{format(debug_info, message)}, has_debug_info_{true} { }

        const char* what() const noexcept override
        {
            return message_.c_str();
        }

        bool has_debug_info() const { return has_debug_info_; }
        void set_debug_info(const Token& debug_info)
        {
            if (has_debug_info_)
                return;
            has_debug_info_ = true;
            message_ = format(debug_info, message_);
        }

    private:
        static string format(const Token& debug_info, const string& message);

        string message_;
        bool has_debug_info_;
    };
}

#endif //FENNEC_COMPILEERROR_H
