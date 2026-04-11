//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_TOKEN_H
#define FENNEC_TOKEN_H

#include "utils/common.h"
#include "TokenType.h"

class Token
{
public:
    Token() = default;
    explicit Token(string lexeme) : Token{init_type(lexeme), std::move(lexeme)} { }
    explicit Token(const TokenType type) : Token{type, type.str()} { }
    Token(const TokenType type, string lexeme) : type_{type}, lexeme_(std::move(lexeme)) { }

    TokenType type() const { return type_; }
    const string& lexeme() const { return lexeme_; }
    const string& filename() const { return filename_; }
    size_t line() const { return line_; }
    basic_t literal() const;

    void set_lexeme(const string& lexeme) { lexeme_ = lexeme; }
    void set_filename(const string& filename) { filename_ = filename; }
    void set_line(const size_t line) { line_ = line; }

    Token instantiate_template_types(const TypeInfoPtr& template_type) const;
    bool empty() const { return lexeme_.empty(); }

    bool operator==(const char c) const { return type_ == c; }
    bool operator==(const string& s) const { return type_ == s; }
    bool operator==(const TokenType t) const { return type_ == t; }
    bool operator==(const TokenType::Enum t) const { return type_ == t; }
    bool operator==(const Token& t) const { return lexeme_ == t.lexeme_; }

    bool operator!=(const char c) const { return not (*this == c); }
    bool operator!=(const string& s) const { return not (*this == s); }
    bool operator!=(const TokenType t) const { return not (*this == t); }
    bool operator!=(const TokenType::Enum t) const { return not (*this == t); }
    bool operator!=(const Token& t) const { return not (*this == t); }

    operator string() const
    {
        return lexeme_;
    }

    static vector<string> to_strings(const vector<Token>& tokens)
    {
        vector<string> strings;
        strings.reserve(tokens.size());
        for (const Token& token : tokens)
            strings.push_back(token.lexeme());
        return strings;
    }

    static string to_string(const optional<Token>& token)
    {
        return token ? token->lexeme() : ""s;
    }

private:
    static TokenType init_type(const string& lexeme);

    TokenType type_;
    string lexeme_;
    string filename_;
    size_t line_ = 0;
};

#endif //FENNEC_TOKEN_H
