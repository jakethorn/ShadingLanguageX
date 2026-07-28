//
// Created by jaket on 02/07/2026.
//

#ifndef MXSLC_PREPROCESS_H
#define MXSLC_PREPROCESS_H

#include "common.h"
#include "Token.h"
#include "CompileOptions.h"
#include "TokenReader.h"

namespace mxslc::preprocess
{
    vector<Token> preprocess(const fs::path& src_path, CompileOptions& opts, bool is_include = false);

    void preprocess(vector<Token>& tokens, CompileOptions& opts);
    void preprocess(vector<Token>& tokens, CompileOptions& opts, bool is_include);
    void preprocess(vector<Token>& tokens, CompileOptions& opts, const optional<fs::path>& src_path);
    void preprocess(vector<Token>& tokens, CompileOptions& opts, const optional<fs::path>& src_path, bool is_include);

    class Preprocessor : protected TokenReader
    {
    public:
        Preprocessor(vector<Token> tokens, CompileOptions& opts, const optional<fs::path>& src_path = std::nullopt, bool is_include = false);
        Preprocessor(const Preprocessor* parent, vector<Token> tokens, CompileOptions& opts, const optional<fs::path>& src_path = std::nullopt, bool is_include = false);

        void preprocess();

        vector<Token> tokens() { return std::move(tokens_); }

    private:
        void process_next_token();
        void process_include();
        void process_library();
        void process_version();
        void process_define();
        void process_undef();
        void process_if();
        void process_if(bool condition);
        void process_if_block();
        bool end_of_if_block() const;
        void process_non_directive();

        void include_file(const fs::path& path);

        void define_macro(const Token& name, vector<Token> body = {}) const;
        void undef_macro(const Token& name) const;
        bool macro_is_defined(const Token& name) const;
        vector<Token> expand_macro(const Token& name) const;
        vector<Token> expand_macros(const vector<Token>& tokens) const;

        vector<Token> consume_and_expand_until(TokenType token_type);

        void define_inclusion_type_macro() const;

        void add_token(Token&& tokens);
        void add_tokens(vector<Token>&& tokens);

        bool is_main() const { return not is_include_; }
        bool is_include() const { return is_include_; }

        bool already_included(const fs::path& path) const;
        void check_for_circular_dependency(const fs::path& path) const;

        const Preprocessor* parent_;
        vector<Token> tokens_;
        CompileOptions& opts_;
        optional<fs::path> path_;
        bool is_include_;

        vector<fs::path> included_files_;

        inline static const string MAIN{"__MAIN__"};
        inline static const string INCLUDE{"__INCLUDE__"};
    };
}

#endif //MXSLC_PREPROCESS_H
