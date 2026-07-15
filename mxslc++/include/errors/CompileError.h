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
        explicit CompileError(const char* message) : message_{message} { }
        explicit CompileError(string message) : message_{std::move(message)} { }
        CompileError(const Token& debug_info, const char* message) : debug_info_{to_string(debug_info)}, message_{message} { }
        CompileError(const Token& debug_info, string message) : debug_info_{to_string(debug_info)}, message_{std::move(message)} { }

        const char* what() const noexcept override
        {
            full_message_ = debug_info_ + message_;
            return full_message_.c_str();
        }

        void set_debug_info(const Token& debug_info)
        {
            if (debug_info_.empty())
                debug_info_ = to_string(debug_info);
        }

        string message() const { return message_; }
        void set_message(string message) { message_ = std::move(message); }

    private:
        static string to_string(const Token& debug_info);

        string debug_info_;
        string message_;
        mutable string full_message_;
    };
}

#endif //FENNEC_COMPILEERROR_H
