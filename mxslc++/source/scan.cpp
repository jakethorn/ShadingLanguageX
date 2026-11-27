//
// Created by jaket on 02/11/2025.
//

#include <regex>
#include <unordered_set>

#include "scan.h"

#include "utils/io_utils.h"
#include "utils/container_utils.h"
#include "errors/CompileError.h"
#include "errors/ErrorReporter.h"

namespace mxslc
{
    using std::regex;
    using std::match_results;
    using string_view_match = std::match_results<string_view::const_iterator>;

    using namespace container_utils;

    namespace
    {
        bool try_match(const TokenType token_type, const regex& pattern, const string_view text, Token& token)
        {
            if (string_view_match match;
                std::regex_search(text.begin(), text.end(), match, pattern, std::regex_constants::match_continuous))
            {
                token = Token{token_type, match[0]};
                return true;
            }

            return false;
        }

        bool try_match_float(const string_view text, Token& token)
        {
            static const regex pattern{R"((([0-9]+\.[0-9]*|\.[0-9]+)([eE][+-]?[0-9]+)?|[0-9]+[eE][+-]?[0-9]+)[fF]?)", std::regex_constants::optimize};
            return try_match(TokenType::Float, pattern, text, token);
        }

        bool try_match_int(const string_view text, Token& token)
        {
            static const regex pattern{R"(\d+)", std::regex_constants::optimize};
            return try_match(TokenType::Int, pattern, text, token);
        }

        bool try_match_string(const string_view text, Token& token)
        {
            static const regex pattern{R"("[^"]*")", std::regex_constants::optimize};
            return try_match(TokenType::String, pattern, text, token);
        }

        bool is_identifier_start_char(const char c)
        {
            return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
        }

        bool is_identifier_char(const char c)
        {
            return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
        }

        bool try_match_bool(const string_view text, Token& token)
        {
            if (text.front() == 't')
            {
                if (text.size() >= 4 && text.compare(0, 4, "true") == 0)
                {
                    if (text.size() >= 5 && is_identifier_char(text[4]))
                        return false;

                    token = Token{TokenType::Bool, string{text.substr(0, 4)}};
                    return true;
                }
            }

            if (text.front() == 'f')
            {
                if (text.size() >= 5 && text.compare(0, 5, "false") == 0)
                {
                    if (text.size() >= 6 && is_identifier_char(text[5]))
                        return false;

                    token = Token{TokenType::Bool, string{text.substr(0, 5)}};
                    return true;
                }
            }

            return false;
        }

        bool try_match_keyword_identifier(const string_view text, Token& token)
        {
            static const regex pattern{R"([_a-zA-Z][_a-zA-Z0-9]*)", std::regex_constants::optimize};
            if (string_view_match match;
                std::regex_search(text.begin(), text.end(), match, pattern, std::regex_constants::match_continuous))
            {
                const TokenType t{match[0]};
                token = Token{t.is_keyword() ? t : TokenType::Identifier, match[0]};
                return true;
            }

            return false;
        }

        bool try_match_comment(const string_view text, Token& token)
        {
            if (text.size() < 2)
                return false;

            if (text[0] != '/')
                return false;

            if (text[1] != '/' and text[1] != '*')
                return false;

            static const regex pattern{R"(/\*[\s\S]*?\*/|//[^\r\n]*)", std::regex_constants::optimize};
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
            if (text.size() > 1)
            {
                if (text[0] == '.' and std::isdigit(text[1]))
                    return false;
            }

            const char c = text.front();
            if (const TokenType t{c}; t.is_symbol())
            {
                token = Token{t, string{c}};
                return true;
            }

            return false;
        }

        bool try_match_token(const string_view text, Token& token)
        {
            return try_match_whitespace(text, token)
                or try_match_compound_symbol(text, token)
                or try_match_comment(text, token)
                or try_match_symbol(text, token)
                or try_match_bool(text, token)
                or try_match_keyword_identifier(text, token)
                or try_match_float(text, token)
                or try_match_int(text, token)
                or try_match_string(text, token);
        }

        Token match_unexpected_characters(const string_view text)
        {
            size_t length = 1;
            Token token;
            while (length < text.size() and not try_match_token(text.substr(length), token))
                ++length;
            return Token{TokenType::Unknown, string{text.substr(0, length)}};
        }
    }

    vector<Token> scan_string(string_view text, const optional<fs::path>& src_path)
    {
        vector<Token> tokens;
        ErrorReporter error_reporter{0};
        size_t line = 1;
        const SharedPathPtr shared_path = copy_to_shared_ptr(src_path);

        while (not text.empty())
        {
            Token token;
            if (not try_match_token(text, token))
                token = match_unexpected_characters(text);
            text.remove_prefix(token.lexeme().length());

            if (token == TokenType::Whitespace)
            {
                continue;
            }

            if (token == TokenType::Comment)
            {
                const string& comment = token.lexeme();
                line += std::count(comment.begin(), comment.end(), '\n');
                continue;
            }

            token.set_line(line);
            token.set_path(shared_path);

            if (token == TokenType::Unknown)
            {
                error_reporter.report(CompileError{token, "Unexpected character: " + token.lexeme()});
                continue;
            }

            if (token == TokenType::Newline)
            {
                ++line;
            }

            tokens.push_back(std::move(token));
        }

        error_reporter.throw_if_errors();
        return tokens;
    }

    vector<Token> scan_file(const fs::path& src_path)
    {
        return scan_string(io_utils::read_file(src_path), src_path);
    }

    vector<Token> scan_source(const SourceRef& source)
    {
        if (source.is_in_memory())
            return scan_string(*source.contents, source.path);
        return scan_file(source.path);
    }
}
