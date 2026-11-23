//
// Created by jaket on 24/11/2025.
//

#include "Token.h"

#include "scan.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "errors/CompileError.h"

namespace mxslc
{
    Token::Token(string lexeme) : lexeme_{std::move(lexeme)}
    {
        type_ = TokenType{lexeme_};
        if (type_ == TokenType::Unknown)
        {
            const vector<Token> tokens = scan_string(lexeme_);
            if (tokens.size() == 1)
                type_ = tokens[0].type();
        }
    }

    Token Token::monomorphize(const TypePtr& template_type) const
    {
        Token t{type(), runtime_utils::monomorphize(lexeme(), template_type)};
        t.set_line(line());
        t.set_path(path_);
        return t;
    }

    string Token::filename() const
    {
        return path_ ? path_->filename().string() : "";
    }

    const fs::path& Token::path() const
    {
        static const fs::path empty_path;
        return path_ ? *path_ : empty_path;
    }

    Primitive Token::literal() const
    {
        if (type_ == TokenType::Bool)
            return lexeme_ == "true";
        if (type_ == TokenType::Int)
            return std::stoi(lexeme_);
        if (type_ == TokenType::Float)
            return std::stof(lexeme_);
        if (type_ == TokenType::String)
            return lexeme_.substr(1, lexeme_.size() - 2);
        throw CompileError{"Invalid literal"};
    }
}
