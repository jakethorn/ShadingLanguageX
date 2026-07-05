//
// Created by jaket on 02/07/2026.
//

#ifndef MXSLC_PREPROCESS_H
#define MXSLC_PREPROCESS_H

#include "Token.h"
#include "CompileOptions.h"
#include "TokenReader.h"

namespace mxslc
{
    void preprocess(std::vector<Token>& tokens, CompileOptions& opts, bool is_include = false);

    class Preprocessor : protected TokenReader
    {
    public:
        Preprocessor(std::vector<Token> tokens, CompileOptions& opts, bool is_include = false);

        void preprocess();

        std::vector<Token> tokens() { return std::move(tokens_); }

    private:
        void process_next_token();
        void process_include();
        void process_library();
        void process_version();
        void process_define();
        void process_undef();
        void process_non_directive();

        void include_file(const std::filesystem::path& path);

        void define_macro(const Token& name, std::vector<Token> body = {}) const;
        void define_macro(const std::string& name, std::vector<Token> body = {}) const;
        void undef_macro(const Token& name) const;
        void undef_macro(const std::string& name) const;
        bool macro_is_defined(const Token& name) const;
        std::vector<Token> evaluate_macro(const Token& name) const;

        void add_token(Token&& tokens);
        void add_tokens(std::vector<Token>&& tokens);

        std::vector<Token> tokens_;
        CompileOptions& opts_;
        bool is_include_;

        static inline std::string MAIN{"__MAIN__"};
        static inline std::string INCLUDE{"__INCLUDE__"};
    };
}

#endif //MXSLC_PREPROCESS_H
