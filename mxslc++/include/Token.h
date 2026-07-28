//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_TOKEN_H
#define FENNEC_TOKEN_H

#include "common.h"
#include "TokenType.h"
#include "Primitive.h"
#include "runtime/utils/monomorphize.h"

namespace mxslc
{
    using runtime_utils::Monomorphizable;

    class Token : public Monomorphizable<Token>, public Stringable
    {
    public:
        Token() = default;
        Token(string lexeme);
        Token(const TokenType type) : Token{type, type.to_string()} { }
        Token(const TokenType type, string lexeme) : type_{type}, lexeme_(std::move(lexeme)) { }

        TokenType type() const { return type_; }
        const string& lexeme() const { return lexeme_; }
        const string& filename() const { return filename_; }
        size_t line() const { return line_; }

        Token monomorphize(const TypePtr& template_type) const override;

        Primitive literal() const;

        template<typename T>
        T literal() const
        {
            return literal().as<T>();
        }

        void set_filename(string filename) { filename_ = std::move(filename); }
        void set_line(const size_t line) { line_ = line; }

        string to_string() const override { return lexeme_; }

        bool operator==(const char c) const { return type_ == c; }
        bool operator==(const TokenType t) const { return type_ == t; }
        bool operator==(const TokenType::Enum t) const { return type_ == t; }
        bool operator==(const string& s) const { return lexeme_ == s; }
        bool operator==(const char* const s) const { return lexeme_ == s; }
        bool operator==(const Token& t) const { return lexeme_ == t.lexeme_; }

        bool operator!=(const char c) const { return not (*this == c); }
        bool operator!=(const TokenType t) const { return not (*this == t); }
        bool operator!=(const TokenType::Enum t) const { return not (*this == t); }
        bool operator!=(const string& s) const { return not (*this == s); }
        bool operator!=(const char* const s) const { return not (*this == s); }
        bool operator!=(const Token& t) const { return not (*this == t); }

        friend bool operator==(const char c, const Token& token) { return token == c; }
        friend bool operator==(const TokenType t, const Token& token) { return token == t; }
        friend bool operator==(const TokenType::Enum t, const Token& token) { return token == t; }
        friend bool operator==(const string& s, const Token& token) { return token == s; }
        friend bool operator==(const char* const s, const Token& token) { return token == s; }

        friend bool operator!=(const char c, const Token& token) { return token != c; }
        friend bool operator!=(const TokenType t, const Token& token) { return token != t; }
        friend bool operator!=(const TokenType::Enum t, const Token& token) { return token != t; }
        friend bool operator!=(const string& s, const Token& token) { return token != s; }
        friend bool operator!=(const char* const s, const Token& token) { return token != s; }

    private:
        TokenType type_;
        string lexeme_;
        string filename_;
        size_t line_{0};
    };
}

#endif //FENNEC_TOKEN_H
