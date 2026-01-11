//
// Created by jaket on 02/11/2025.
//

#include "scan.h"
#include <regex>
#include <unordered_set>

#include "CompileError.h"
#include "utils/io_utils.h"
#include "utils/template_utils.h"

using std::regex;
using std::match_results;
using string_view_match = std::match_results<string_view::const_iterator>;

namespace
{
    bool try_match(const TokenType token_type, const regex& pattern, const string_view text, Token& token)
    {
        if (string_view_match match;
            std::regex_search(text.begin(), text.end(), match, pattern))
        {
            token = Token{token_type, match[0]};
            return true;
        }

        return false;
    }

    bool try_match_float(const string_view text, Token& token)
    {
        static const regex pattern{R"(^(([0-9]*\.[0-9]+)|([0-9]+\.[0-9]*))(e-?[0-9]+)?)"};
        return try_match(TokenType::Float, pattern, text, token);
    }

    bool try_match_int(const string_view text, Token& token)
    {
        static const regex pattern{R"(^\d+)"};
        return try_match(TokenType::Int, pattern, text, token);
    }

    bool try_match_string(const string_view text, Token& token)
    {
        static const regex pattern{R"(^"[^"]*")"};
        return try_match(TokenType::String, pattern, text, token);
    }

    bool try_match_bool(const string_view text, Token& token)
    {
        static const regex pattern{R"(^(true|false))"};
        return try_match(TokenType::Bool, pattern, text, token);
    }

    bool try_match_keyword_identifier(const string_view text, Token& token)
    {
        static const regex pattern{R"(^[_a-zA-Z][_a-zA-Z0-9]*)"};
        if (string_view_match match;
            std::regex_search(text.begin(), text.end(), match, pattern))
        {
            const TokenType t{match[0]};
            token = Token{t.is_keyword() ? t : TokenType::Identifier, match[0]};
            return true;
        }

        return false;
    }

    bool try_match_comment(const string_view text, Token& token)
    {
        static const regex pattern{R"(^//.*)"};
        return try_match(TokenType::Comment, pattern, text, token);
    }

    bool try_match_whitespace(const string_view text, Token& token)
    {
        static const unordered_set whitespace{' ', '\r', '\t', '\n'};

        if (const char c = text.front(); contains(whitespace, c))
        {
            const TokenType t{c == '\n' ? TokenType::Newline : TokenType::Whitespace};
            token = Token{t, string{c}};
            return true;
        }

        return false;
    }

    bool try_match_compound_symbol(const string_view text, Token& token)
    {
        if (text.size() < 2)
            return false;

        string s{text.substr(0, 2)};
        if (const TokenType t{s}; t.is_compound_symbol())
        {
            token = Token{t, std::move(s)};
            return true;
        }

        return false;
    }

    bool try_match_symbol(const string_view text, Token& token)
    {
        const char c = text.front();
        if (const TokenType t{c}; t.is_symbol())
        {
            token = Token{t, string{c}};
            return true;
        }

        return false;
    }

    Token next_token(const string_view text, const size_t line)
    {
        if (Token token; try_match_whitespace(text, token)
                         or try_match_comment(text, token)
                         or try_match_float(text, token)
                         or try_match_int(text, token)
                         or try_match_string(text, token)
                         or try_match_bool(text, token)
                         or try_match_keyword_identifier(text, token)
                         or try_match_compound_symbol(text, token)
                         or try_match_symbol(text, token))
        {
            token.set_line(line);
            return token;
        }

        throw CompileError{"Scanning error on line "s + std::to_string(line) + ", character: "s + text.front()};
    }
}

vector<Token> sscan(string_view text)
{
    vector<Token> tokens;
    size_t line = 1;

    while (not text.empty())
    {
        Token token = next_token(text, line);
        text.remove_prefix(token.lexeme().length());

        if (token == TokenType::Whitespace or token == TokenType::Comment)
        {
            continue;
        }

        if (token == TokenType::Newline)
        {
            ++line;
            continue;
        }

        tokens.push_back(token);
    }

    return tokens;
}

vector<Token> fscan(const fs::path& filepath)
{
    const string text = read_file(filepath);
    vector<Token> tokens = sscan(text);
    const string filename = filepath.filename().string();
    for (Token& token : tokens)
    {
        token.set_filename(filename);
    }
    return tokens;
}
