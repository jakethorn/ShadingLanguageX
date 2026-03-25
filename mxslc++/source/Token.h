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
    Token(string lexeme) : Token{init_type_(lexeme), std::move(lexeme)} { }
    Token(const TokenType type) : Token{type, type.str()} { }
    Token(const TokenType type, string lexeme) : type_{type}, lexeme_(std::move(lexeme)) { }

    [[nodiscard]] TokenType type() const { return type_; }
    [[nodiscard]] const string& lexeme() const { return lexeme_; }
    [[nodiscard]] const string& filename() const { return filename_; }
    [[nodiscard]] size_t line() const { return line_; }
    [[nodiscard]] basic_t literal() const;

    void set_line(const size_t line) { line_ = line; }
    void set_filename(const string& filename) { filename_ = filename; }
    void set_lexeme(const string& lexeme) { lexeme_ = lexeme; }

    [[nodiscard]] Token instantiate_template_types(const TypeInfoPtr& type) const;
    [[nodiscard]] bool empty() const { return lexeme_.empty(); }

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

    static vector<string> as_strings(const vector<Token>& tokens)
    {
        vector<string> strings;
        strings.reserve(tokens.size());
        for (const Token& token : tokens)
            strings.push_back(token.lexeme());
        return strings;
    }

private:
    static TokenType init_type_(const string& lexeme);

    TokenType type_;
    string lexeme_;
    string filename_;
    size_t line_ = 0;
};

#endif //FENNEC_TOKEN_H
