//
// Created by jaket on 24/11/2025.
//

#include <cassert>
#include "Token.h"
#include "scan.h"
#include "runtime/Type.h"
#include "runtime/TypeInfo.h"

TokenType Token::init_type_(const string& lexeme)
{
    const vector tokens = sscan(lexeme);
    assert(tokens.size() == 1);
    return tokens[0].type();
}

basic_t Token::literal() const
{
    assert(
        type_ == TokenType::Bool or
        type_ == TokenType::Int or
        type_ == TokenType::Float or
        type_ == TokenType::String
    );

    if (type_ == TokenType::Bool)
        return lexeme_ == "true";
    if (type_ == TokenType::Int)
        return std::stoi(lexeme_);
    if (type_ == TokenType::Float)
        return std::stof(lexeme_);
    return lexeme_.substr(1, lexeme_.size() - 2);
}

Token Token::instantiate_template_types(const TypeInfoPtr& type) const
{
    Token t{type_, lexeme_ == "T" ? type->name() : lexeme_};
    t.set_line(line_);
    t.set_filename(filename_);
    return t;
}
