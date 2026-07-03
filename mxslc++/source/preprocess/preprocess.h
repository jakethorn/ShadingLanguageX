//
// Created by jaket on 02/07/2026.
//

#ifndef MXSLC_PREPROCESS_H
#define MXSLC_PREPROCESS_H

#include "TokenReader.h"
#include "utils/common.h"

void preprocess(vector<Token>& tokens, CompileOptions& opts);

class Preprocessor final : protected TokenReader
{
public:
    explicit Preprocessor(vector<Token> tokens, CompileOptions& opts);

    void preprocess();

    vector<Token> tokens() { return std::move(tokens_); }

private:
    void process_next_token();
    void process_include();
    void process_library();
    void process_define();
    void process_non_directive();

    void include_file(const fs::path& path);

    vector<Token> tokens_;
    CompileOptions& opts_;
};

#endif //MXSLC_PREPROCESS_H
