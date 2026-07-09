//
// Created by jaket on 02/07/2026.
//

#ifndef MXSLC_PREPROCESS_H
#define MXSLC_PREPROCESS_H

#include "common.h"
#include "Token.h"
#include "CompileOptions.h"
#include "Primitive.h"
#include "TokenReader.h"

namespace mxslc::preprocess
{
    void preprocess(vector<Token>& tokens, CompileOptions& opts, bool is_include = false);

    class Preprocessor : protected TokenReader
    {
    public:
        Preprocessor(vector<Token> tokens, CompileOptions& opts, bool is_include = false);

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
        void process_non_directive();

        void include_file(const fs::path& path);

        void define_macro(const Token& name, vector<Token> body = {}) const;
        void define_macro(const string& name, vector<Token> body = {}) const;
        void undef_macro(const Token& name) const;
        void undef_macro(const string& name) const;
        bool macro_is_defined(const Token& name) const;
        vector<Token> expand_macro(const Token& name) const;

        vector<Token> consume_and_expand_until(TokenType token_type);

        Primitive evaluate_expression(const vector<Token>& tokens) const;

        void add_token(Token&& tokens);
        void add_tokens(vector<Token>&& tokens);

        vector<Token> tokens_;
        CompileOptions& opts_;
        bool is_include_;

        inline static const string MAIN{"__MAIN__"};
        inline static const string INCLUDE{"__INCLUDE__"};
    };
}

#endif //MXSLC_PREPROCESS_H
