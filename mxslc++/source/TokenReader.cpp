//
// Created by jaket on 13/11/2025.
//

#include "TokenReader.h"

#include "common.h"
#include "utils/container_utils.h"

namespace mxslc
{
    using namespace container_utils;

    size_t TokenReader::size() const
    {
        return tokens_.size() - index_;
    }

    bool TokenReader::empty() const
    {
        return index_ == tokens_.size();
    }

    void TokenReader::ignore(const TokenType token_type)
    {
        tokens_.erase(
            std::remove(tokens_.begin(), tokens_.end(), token_type),
            tokens_.end()
        );
    }

    const Token& TokenReader::peek(const size_t n) const
    {
        check_bounds(n);
        return tokens_[index_ + n];
    }

    Span<Token> TokenReader::peek_until(const TokenType type, size_t max_tokens) const
    {
        max_tokens = std::min(max_tokens, size());
        for (size_t i = 0; i < max_tokens; ++i)
        {
            if (tokens_[index_ + i] == type)
            {
                return Span{tokens_, index_, i+1};
            }
        }

        return Span{tokens_, index_, max_tokens};
    }

    Token TokenReader::consume()
    {
        check_bounds();
        return std::move(tokens_[index_++]);
    }

    optional<Token> TokenReader::consume(const vector<TokenType>& types)
    {
        if (empty())
            return std::nullopt;
        if (contains(types, peek().type()))
            return std::move(tokens_[index_++]);
        return std::nullopt;
    }

    Token TokenReader::match_identifier_or_keyword()
    {
        Token token = consume();
        if (token == TokenType::Identifier or token.type().is_keyword())
            return token;
        throw CompileError{token, "Unexpected token: " + token.lexeme()};
    }

    void TokenReader::check_bounds(const size_t n) const
    {
        if (index_ + n >= tokens_.size())
            throw CompileError{tokens_.back(), "Unexpected end of tokens"s};
    }
}
