//
// Created by jaket on 13/11/2025.
//

#ifndef FENNEC_TOKENREADER_H
#define FENNEC_TOKENREADER_H

#include <algorithm>
#include <limits>
#include "utils/common.h"
#include "CompileError.h"
#include "Token.h"
#include "TokenType.h"
#include "Span.h"

using namespace mxslc;

class TokenReader
{
public:
    TokenReader() = default;
    explicit TokenReader(vector<Token> tokens) : tokens_{std::move(tokens)} { }

    [[nodiscard]] size_t size() const;
    [[nodiscard]] bool empty() const;

    [[nodiscard]] const Token& peek(size_t n = 0) const;
    [[nodiscard]] Span<Token> peek_until(TokenType type, size_t max_tokens = std::numeric_limits<size_t>::max()) const;

    Token consume();

    template<typename... Args>
    optional<Token> consume(const Args&... types)
    {
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

        throw CompileError{token, "Unexpected token: "s + token.lexeme()};
    }

private:
    void check_bounds(size_t n = 0) const;

    vector<Token> tokens_;
    size_t index_ = 0;
};

#endif //FENNEC_TOKENREADER_H
