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
                 if (token == "include") process_include();
            else if (token == "library") process_library();
            else if (token == "version") process_version();
            else if (token == "define")  process_define();
            else if (token == "undef")   process_undef();
            else if (token == "if")      process_if();
            else throw CompileError{"Unknown preprocessor directive: " + token.lexeme()};
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
        const Token name = match(TokenType::Identifier);
        vector<Token> body = consume_until(TokenType::Newline);
        define_macro(name, std::move(body));
    }

    void Preprocessor::process_undef()
    {
        const Token name = match(TokenType::Identifier);
        undef_macro(name);
    }

    void Preprocessor::process_if()
    {
        vector<Token> tokens = consume_and_expand_until(TokenType::Newline);
        const bool condition = parse_primitive(std::move(tokens)).cast<bool>();
        process_if(condition);
    }

    void Preprocessor::process_if(const bool condition)
    {
        if (condition)
        {
            process_if_block();
            match(TokenType::Hash);
            const string& directive = consume().lexeme();
        }
        else
        {
            consume_if_block();
        }
    }

    void Preprocessor::process_if_block()
    {
        while (not end_of_if_block())
            process_next_token();
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
        opts_.macros.emplace_back(name.lexeme(), std::move(body));
    }

    void Preprocessor::undef_macro(const Token& name) const
    {
        const auto it = position_of(opts_.macros, name);
        if (it != opts_.macros.end())
            opts_.macros.erase(it);
    }

    bool Preprocessor::macro_is_defined(const Token& name) const
    {
        return contains(opts_.macros, name);
    }

    vector<Token> Preprocessor::expand_macro(const Token& name) const
    {
        const auto it = position_of(opts_.macros, name);
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
