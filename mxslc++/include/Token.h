//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_TOKEN_H
#define FENNEC_TOKEN_H

#include "common.h"
#include "primitive_t.h"
#include "TokenType.h"

namespace mxslc
{
    class Token
    {
    public:
        Token() = default;
        explicit Token(string lexeme);
        explicit Token(const TokenType type) : Token{type, type.str()} { }
        Token(const TokenType type, string lexeme) : type_{type}, lexeme_(std::move(lexeme)) { }

        TokenType type() const { return type_; }
        const string& lexeme() const { return lexeme_; }
        const string& filename() const { return filename_; }
        size_t line() const { return line_; }

        primitive_t literal() const;

        template<typename T>
        T literal() const
        {
            return std::get<T>(literal());
        }

        void set_filename(string filename) { filename_ = std::move(filename); }
        void set_line(const size_t line) { line_ = line; }

        bool operator==(const char c) const { return type_ == c; }
        bool operator==(const TokenType t) const { return type_ == t; }
        bool operator==(const TokenType::Enum t) const { return type_ == t; }
        bool operator==(const string& s) const { return lexeme_ == s; }
        bool operator==(const Token& t) const { return lexeme_ == t.lexeme_; }

        bool operator!=(const char c) const { return not (*this == c); }
        bool operator!=(const TokenType t) const { return not (*this == t); }
        bool operator!=(const TokenType::Enum t) const { return not (*this == t); }
        bool operator!=(const string& s) const { return not (*this == s); }
        bool operator!=(const Token& t) const { return not (*this == t); }

    private:
        TokenType type_;
        string lexeme_;
        string filename_;
        size_t line_{0};
    };
}

#endif //FENNEC_TOKEN_H
