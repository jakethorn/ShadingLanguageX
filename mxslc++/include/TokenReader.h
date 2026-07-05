//
// Created by jaket on 13/11/2025.
//

#ifndef FENNEC_TOKENREADER_H
#define FENNEC_TOKENREADER_H

#include <algorithm>
#include <limits>
#include <optional>

#include "CompileError.h"
#include "Token.h"
#include "TokenType.h"
#include "Span.h"

namespace mxslc
{
    class TokenReader
    {
    public:
        TokenReader() = default;
        explicit TokenReader(std::vector<Token> tokens) : tokens_{std::move(tokens)} { }

        size_t size() const;
        bool empty() const;

        void ignore(TokenType token_type);

        const Token& peek(size_t n = 0) const;
        Span<Token> peek_until(TokenType type, size_t max_tokens = std::numeric_limits<size_t>::max()) const;

        Token consume();
        std::optional<Token> consume(const std::vector<TokenType>& types);

        template<typename... Args>
        std::optional<Token> consume(const Args&... types)
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
        std::vector<Token> consume_while(const Args&... types)
        {
            std::vector<Token> tokens;
            while (std::optional<Token> token = consume(types...))
            {
                tokens.push_back(std::move(*token));
            }

            return tokens;
        }

        template<typename... Args>
        std::vector<Token> consume_until(const Args&... types)
        {
            std::vector<Token> tokens;
            while ((... || (peek() != types)))
            {
                tokens.push_back(consume());
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

            throw CompileError{token, "Unexpected token: " + token.lexeme()};
        }

        Token match_identifier_or_keyword();

    private:
        void check_bounds(size_t n = 0) const;

        std::vector<Token> tokens_;
        size_t index_ = 0;
    };
}

#endif //FENNEC_TOKENREADER_H
