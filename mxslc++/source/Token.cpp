//
// Created by jaket on 24/11/2025.
//

#include <cassert>
#include "Token.h"

#include "CompileError.h"
#include "scan.h"
#include "runtime/TypeInfo.h"
#include "utils/instantiate_template_types_utils.h"

TokenType Token::init_type(const string& lexeme)
{
    const vector<Token> tokens = sscan(lexeme);
    assert(tokens.size() == 1);
    return tokens[0].type();
}

basic_t Token::literal() const
{
    if (type_ == TokenType::Bool)
        return lexeme_ == "true";
    if (type_ == TokenType::Int)
        return std::stoi(lexeme_);
    if (type_ == TokenType::Float)
        return std::stof(lexeme_);
    if (type_ == TokenType::String)
        return lexeme_.substr(1, lexeme_.size() - 2);
    throw CompileError{"Invalid literal"s};
}

Token Token::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    Token t{type_, ::instantiate_template_types(lexeme_, template_type)};
    t.set_line(line_);
    t.set_filename(filename_);
    return t;
}
