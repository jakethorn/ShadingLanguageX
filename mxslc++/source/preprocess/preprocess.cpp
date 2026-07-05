//
// Created by jaket on 02/07/2026.
//

#include "preprocess.h"
#include "scan.h"
#include "utils/template_utils.h"
#include "utils/token_utils.h"

namespace mxslc
{
    void preprocess(vector<Token>& tokens, CompileOptions& opts, const bool is_include)
    {
        Preprocessor preprocessor{std::move(tokens), opts, is_include};
        preprocessor.preprocess();
        tokens = preprocessor.tokens();
    }

    Preprocessor::Preprocessor(vector<Token> tokens, CompileOptions& opts, const bool is_include) : TokenReader{std::move(tokens)}, opts_{opts}, is_include_{is_include}
    {

    }

    void Preprocessor::preprocess()
    {
        if (is_include_)
        {
            undef_macro(MAIN);
            define_macro(INCLUDE);
        }
        else
        {
            define_macro(MAIN);
            undef_macro(INCLUDE);
        }

        for (const fs::path& path : opts_.includes)
            include_file(path);

        while (not empty())
            process_next_token();

        undef_macro(MAIN);
        undef_macro(INCLUDE);
    }

    void Preprocessor::process_next_token()
    {
        if (consume(TokenType::Hash))
        {
            static const std::unordered_map<string, void (Preprocessor::*)()> directives = {
                {"include", &Preprocessor::process_include},
                {"library", &Preprocessor::process_library},
                {"version", &Preprocessor::process_version},
                {"define", &Preprocessor::process_define},
                {"undef", &Preprocessor::process_undef},
            };

            const string& lexeme = consume().lexeme();
            const auto directive = directives.find(lexeme);
            if (directive != directives.end())
                (this->*directive->second)();
            else
                throw CompileError{"Unknown preprocessor directive: " + lexeme};
        }
        else
        {
            process_non_directive();
        }
    }

    void Preprocessor::process_include()
    {
        // TODO use search paths when getting path
        // TODO track which files have been included
        // TODO track circular dependencies (look at ancestors)
        const Token path = match(TokenType::String);
        include_file(path.literal<string>());
    }

    void Preprocessor::process_library()
    {
        const Token path = match(TokenType::String);
        opts_.libraries.emplace_back(path.literal<string>());
    }

    void Preprocessor::process_version()
    {
        const vector<Token> version_parts = consume_until(TokenType::Newline);
        opts_.version = utils::join_tokens(version_parts);
    }

    void Preprocessor::process_define()
    {
        const Token name = match(TokenType::Identifier);
        vector<Token> body = consume_until(TokenType::Newline);
        define_macro(name, std::move(body));
    }

    void Preprocessor::process_undef()
    {
        const Token name = match(TokenType::Identifier);
        undef_macro(name);
    }

    void Preprocessor::process_non_directive()
    {
        Token token = consume();

        if (token == TokenType::Identifier and macro_is_defined(token))
        {
            vector<Token> tokens = evaluate_macro(token);
            add_tokens(std::move(tokens));
            return;
        }

        if (token != TokenType::Newline)
            add_token(std::move(token));
    }

    void Preprocessor::include_file(const fs::path& path)
    {
        vector<Token> included_tokens = scan_file(path);
        mxslc::preprocess(included_tokens, opts_, true);
        add_tokens(std::move(included_tokens));

        if (not is_include_)
            define_macro(MAIN);
    }

    void Preprocessor::define_macro(const Token& name, vector<Token> body) const
    {
        if (macro_is_defined(name))
            undef_macro(name);
        opts_.macros.emplace_back(name.lexeme(), std::move(body));
    }

    void Preprocessor::define_macro(const std::string& name, std::vector<Token> body) const
    {
        define_macro({TokenType::Identifier, name}, std::move(body));
    }

    void Preprocessor::undef_macro(const Token& name) const
    {
        const auto it = position_of(opts_.macros, name);
        if (it != opts_.macros.end())
            opts_.macros.erase(it);
    }

    void Preprocessor::undef_macro(const std::string& name) const
    {
        undef_macro({TokenType::Identifier, name});
    }

    bool Preprocessor::macro_is_defined(const Token& name) const
    {
        return contains(opts_.macros, name);
    }

    vector<Token> Preprocessor::evaluate_macro(const Token& name) const
    {
        for (const Macro& macro : opts_.macros)
        {
            if (macro == name)
            {
                vector<Token> tokens = macro.body();
                mxslc::preprocess(tokens, opts_, is_include_);
                return tokens;
            }
        }

        throw CompileError{name, "Macro not defined: " + name.lexeme()};
    }

    void Preprocessor::add_token(Token&& tokens)
    {
        tokens_.emplace_back(std::move(tokens));
    }

    void Preprocessor::add_tokens(std::vector<Token>&& tokens)
    {
        tokens_.insert(tokens_.cend(), std::make_move_iterator(tokens.begin()), std::make_move_iterator(tokens.end()));
    }
}

