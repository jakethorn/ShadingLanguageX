//
// Created by jaket on 13/11/2025.
//

#include "TokenReader.h"
#include "CompileError.h"

size_t TokenReader::size() const
{
    return tokens_.size() - index_;
}

bool TokenReader::empty() const
{
    return index_ == tokens_.size();
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

void TokenReader::check_bounds(const size_t n) const
{
    if (index_ + n >= tokens_.size())
    {
        throw CompileError{tokens_.back(), "Unexpected end of tokens"s};
    }
}
