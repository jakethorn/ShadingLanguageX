//
// Created by jaket on 13/11/2025.
//

#ifndef FENNEC_TOKENREADER_H
#define FENNEC_TOKENREADER_H

#include <algorithm>
#include <limits>
#include "utils/common.h"
#include "Token.h"
#include "TokenType.h"
#include "Span.h"

class TokenReader
{
public:
    TokenReader() = default;
    explicit TokenReader(vector<Token> tokens) : tokens_{std::move(tokens)} { }

    size_t size() const;
    bool empty() const;

    const Token& peek(size_t n = 0) const;
    Span<Token> peek_until(TokenType type, size_t max_tokens = std::numeric_limits<size_t>::max()) const;

    Token consume();

    template<typename... Args>
    optional<Token> consume(const Args&... types)
    {
        if (empty())
            return std::nullopt;

        const Token& token = peek();
        if ((... || (token == types)))
        {
            return std::move(tokens_[index_++]);
        }

        return std::nullopt;
    }

    template<typename... Args>
    vector<Token> consume_while(const Args&... types)
    {
        vector<Token> tokens;
        while (optional<Token> token = consume(types...))
        {
            tokens.push_back(std::move(*token));
        }

        return tokens;
    }

    template<typename... Args>
    Token match(const Args&... types)
    {
        const Token& token = peek();
        if ((... || (token == types)))
        {
            return std::move(tokens_[index_++]);
        }

        throw_error(token, "Unexpected token: " + token.lexeme());
        return {};
    }

    Token match_identifier_or_keyword();

private:
    void check_bounds(size_t n = 0) const;
    static void throw_error(const Token& token, const string& message);

    vector<Token> tokens_;
    size_t index_ = 0;
};

#endif //FENNEC_TOKENREADER_H
