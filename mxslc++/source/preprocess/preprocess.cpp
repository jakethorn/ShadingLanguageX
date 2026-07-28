//
// Created by jaket on 02/07/2026.
//

#include "preprocess/preprocess.h"
#include "scan.h"
#include "preprocess/parse_primitive.h"
#include "utils/container_utils.h"
#include "utils/token_utils.h"
#include "utils/io_utils.h"

namespace mxslc::preprocess
{
    using container_utils::contains;
    using container_utils::position_of;
    using container_utils::extend;

    vector<Token> preprocess(const fs::path& src_path, CompileOptions& opts, const bool is_include)
    {
        vector<Token> included_tokens = scan_file(src_path);
        preprocess(included_tokens, opts, src_path, is_include);
        return included_tokens;
    }

    void preprocess(vector<Token>& tokens, CompileOptions& opts)
    {
        preprocess(tokens, opts, std::nullopt, false);
    }

    void preprocess(vector<Token>& tokens, CompileOptions& opts, const bool is_include)
    {
        preprocess(tokens, opts, std::nullopt, is_include);
    }

    void preprocess(vector<Token>& tokens, CompileOptions& opts, const optional<fs::path>& src_path)
    {
        preprocess(tokens, opts, src_path, false);
    }

    void preprocess(vector<Token>& tokens, CompileOptions& opts, const optional<fs::path>& src_path, const bool is_include)
    {
        Preprocessor preprocessor{std::move(tokens), opts, src_path, is_include};
        preprocessor.preprocess();
        tokens = preprocessor.tokens();
    }

    Preprocessor::Preprocessor(vector<Token> tokens, CompileOptions& opts, const optional<fs::path>& src_path, const bool is_include)
        : TokenReader{std::move(tokens)}, parent_{nullptr}, opts_{opts}, path_{src_path}, is_include_{is_include}
    {

    }

    Preprocessor::Preprocessor(const Preprocessor* parent, vector<Token> tokens, CompileOptions& opts, const optional<fs::path>& src_path, const bool is_include)
        : TokenReader{std::move(tokens)}, parent_{parent}, opts_{opts}, path_{src_path}, is_include_{is_include}
    {

    }

    void Preprocessor::preprocess()
    {
        define_inclusion_type_macro();

        if (is_main())
        {
            for (const fs::path& path : opts_.includes)
                include_file(path);
        }

        while (not empty())
            process_next_token();
    }

    void Preprocessor::process_next_token()
    {
        if (consume(TokenType::Hash))
        {
            const Token token = consume();
                 if (token == "include")  process_include();
            else if (token == "library")  process_library();
            else if (token == "version")  process_version();
            else if (token == "define")   process_define();
            else if (token == "undef")    process_undef();
            else if (token == "if")       process_if();
            else if (token == "ifdef")    process_ifdef();
            else if (token == "ifndef")   process_ifndef();
            else if (token == "elif")     throw CompileError{token, "#elif without starting #(if/ifdef/ifndef)"};
            else if (token == "elifdef")  throw CompileError{token, "#elifdef without starting #(if/ifdef/ifndef)"};
            else if (token == "elifndef") throw CompileError{token, "#elifndef without starting #(if/ifdef/ifndef)"};
            else if (token == "else")     throw CompileError{token, "#else without starting #(if/ifdef/ifndef)"};
            else if (token == "endif")     throw CompileError{token, "#endif without starting #(if/ifdef/ifndef)"};
            else throw CompileError{token, "Unknown preprocessor directive: " + token.lexeme()};
        }
        else
        {
            process_non_directive();
        }
    }

    void Preprocessor::process_include()
    {
        vector<Token> tokens = consume_and_expand_until(TokenType::Newline);
        const fs::path path = parse_primitive(std::move(tokens)).cast<fs::path>();
        include_file(path);
    }

    void Preprocessor::process_library()
    {
        vector<Token> tokens = consume_and_expand_until(TokenType::Newline);
        fs::path path = parse_primitive(std::move(tokens)).cast<fs::path>();
        opts_.libraries.push_back(std::move(path));
    }

    void Preprocessor::process_version()
    {
        const vector<Token> version_parts = consume_and_expand_until(TokenType::Newline);
        opts_.version = token_utils::join_tokens(version_parts);
    }

    void Preprocessor::process_define()
    {
        const Token name = match_macro();
        vector<Token> body = consume_until(TokenType::Newline);
        define_macro(name, std::move(body));
    }

    void Preprocessor::process_undef()
    {
        undef_macro(match_macro());
    }

    void Preprocessor::process_if(const bool ignore)
    {
        vector<Token> tokens = consume_and_expand_until(TokenType::Newline);
        const bool condition = parse_primitive(std::move(tokens)).cast<bool>();
        process_remaining_if_blocks(condition, ignore);
    }

    void Preprocessor::process_ifdef(const bool ignore)
    {
        process_remaining_if_blocks(macro_is_defined(match_macro()), ignore);
    }

    void Preprocessor::process_ifndef(const bool ignore)
    {
        process_remaining_if_blocks(not macro_is_defined(match_macro()), ignore);
    }

    void Preprocessor::process_remaining_if_blocks(const bool condition, const bool ignore)
    {
        if (condition and not ignore)
        {
            process_if_block();

            const Token directive = match_directive();
            if (directive == "elif")
                process_if(true);
            else if (directive == "elifdef")
                process_ifdef(true);
            else if (directive == "elifndef")
                process_ifndef(true);
            else if (directive == "else")
                process_else_block(true);
        }
        else
        {
            consume_if_block();

            const Token directive = match_directive();
            if (directive == "elif")
                process_if(ignore);
            else if (directive == "elifdef")
                process_ifdef(ignore);
            else if (directive == "elifndef")
                process_ifndef(ignore);
            else if (directive == "else")
                process_else_block(ignore);
        }
    }

    void Preprocessor::process_else_block(const bool ignore)
    {
        if (not ignore)
            process_if_block();
        else
            consume_if_block();

        const Token directive = match_directive();
        if (directive != "endif")
            throw CompileError{directive, "Encountered more conditional directives after #else"};
    }

    void Preprocessor::process_if_block()
    {
        while (not end_of_if_block())
            process_next_token();
    }

    void Preprocessor::consume_if_block()
    {
        while (not end_of_if_block())
        {
            // nested if directive
            if (start_of_if_block())
            {
                const Token directive = match_directive();
                if (directive == "if")
                    process_if(/*ignore*/true);
                else if (directive == "ifdef")
                    process_ifdef(/*ignore*/true);
                else if (directive == "ifndef")
                    process_ifndef(/*ignore*/true);
            }
            else
            {
                consume();
            }
        }
    }

    bool Preprocessor::start_of_if_block() const
    {
        return peek() == TokenType::Hash and contains(vector{"if", "ifdef", "ifndef"}, peek(1));
    }

    bool Preprocessor::end_of_if_block() const
    {
        return peek() == TokenType::Hash and contains(vector{"elif", "elifdef", "elifndef", "else", "endif"}, peek(1));
    }

    void Preprocessor::process_non_directive()
    {
        Token token = consume();

        if (macro_is_defined(token))
        {
            vector<Token> tokens = expand_macro(token);
            add_tokens(std::move(tokens));
            return;
        }

        if (token != TokenType::Newline)
            add_token(std::move(token));
    }

    void Preprocessor::include_file(const fs::path& path)
    {
        io_utils::search(opts_.search_directories(), path, [this](const fs::path& found_path) {
            check_for_circular_dependency(found_path);
            if (already_included(found_path))
                return;
            vector<Token> tokens = scan_file(found_path);
            Preprocessor preprocessor{this, std::move(tokens), opts_, found_path, true};
            preprocessor.preprocess();
            add_tokens(preprocessor.tokens());
            included_files_.push_back(found_path);
            extend(included_files_, std::move(preprocessor.included_files_));
            define_inclusion_type_macro();
        });
    }

    void Preprocessor::define_macro(const Token& name, vector<Token> body) const
    {
        if (macro_is_defined(name))
            undef_macro(name);
        opts_.macros.emplace(name.lexeme(), std::move(body));
    }

    void Preprocessor::undef_macro(const Token& name) const
    {
        opts_.macros.erase(name.lexeme());
    }

    bool Preprocessor::macro_is_defined(const Token& name) const
    {
        return contains(opts_.macros, name.lexeme());
    }

    vector<Token> Preprocessor::expand_macro(const Token& name) const
    {
        const auto it = opts_.macros.find(name.lexeme());
        if (it != opts_.macros.end())
        {
            return expand_macros(it->body());
        }

        throw CompileError{name, "Macro not defined: " + name.lexeme()};
    }

    vector<Token> Preprocessor::expand_macros(const vector<Token>& tokens) const
    {
        vector<Token> result;
        result.reserve(tokens.size());
        for (const Token& token : tokens)
        {
            if (macro_is_defined(token))
                extend(result, expand_macro(token));
            else
                result.push_back(token);
        }

        return result;
    }

    vector<Token> Preprocessor::consume_and_expand_until(const TokenType token_type)
    {
        return expand_macros(consume_until(token_type));
    }

    void Preprocessor::define_inclusion_type_macro() const
    {
        undef_macro(MAIN);
        undef_macro(INCLUDE);

        if (is_main())
            define_macro(MAIN);
        else
            define_macro(INCLUDE);
    }

    void Preprocessor::add_token(Token&& tokens)
    {
        tokens_.emplace_back(std::move(tokens));
    }

    void Preprocessor::add_tokens(vector<Token>&& tokens)
    {
        tokens_.insert(tokens_.cend(), std::make_move_iterator(tokens.begin()), std::make_move_iterator(tokens.end()));
    }

    bool Preprocessor::already_included(const fs::path& path) const
    {
        if (contains(included_files_, path))
            return true;

        if (parent_)
            return parent_->already_included(path);

        return false;
    }

    void Preprocessor::check_for_circular_dependency(const fs::path& path) const
    {
        if (path_ == path)
            throw CompileError{"Circular dependency detected in " + path_->filename().string() + " when including " + path.filename().string()};

        if (parent_)
            parent_->check_for_circular_dependency(path);
    }
}
