//
// Created by jaket on 02/07/2026.
//

#include "preprocess.h"

#include "scan.h"

void preprocess(vector<Token>& tokens, CompileOptions& opts)
{
    Preprocessor preprocessor{std::move(tokens), opts};
    preprocessor.preprocess();
    tokens = preprocessor.tokens();
}

Preprocessor::Preprocessor(vector<Token> tokens, CompileOptions& opts) : TokenReader{std::move(tokens)}, opts_{opts}
{

}

void Preprocessor::preprocess()
{
    for (const fs::path& path : opts_.includes)
        include_file(path);

    while (not empty())
        process_next_token();
}

void Preprocessor::process_next_token()
{
    static const std::unordered_map<TokenType, void (Preprocessor::*)()> directives = {
        {TokenType::HashInclude, &Preprocessor::process_include},
        {TokenType::HashLibrary, &Preprocessor::process_library},
        {TokenType::HashDefine, &Preprocessor::process_define},
    };

    auto directive = directives.find(peek().type());
    if (directive != directives.end())
        (this->*directive->second)();
    else
        process_non_directive();
}

void Preprocessor::process_include()
{
    // TODO use search paths when getting path
    // TODO undef __MAIN__
    // TODO define __INCLUDE__
    // TODO pass defines to sub ::preprocess
    // TODO track which files have been included
    // TODO track circular dependencies (look at ancestors)
    match(TokenType::HashInclude);
    const Token path = match(TokenType::String);
    match(TokenType::Newline);

    include_file(path.literal<string>());
}

void Preprocessor::process_library()
{
    match(TokenType::HashLibrary);
    const Token path = match(TokenType::String);
    match(TokenType::Newline);

    opts_.libraries.emplace_back(path.literal<string>());
}

void Preprocessor::process_define()
{
    match(TokenType::HashDefine);
    vector<Token> body = consume_until(TokenType::Newline);
    match(TokenType::Newline);
}

void Preprocessor::process_non_directive()
{
    Token token = consume();
    if (token != TokenType::Newline)
        tokens_.emplace_back(std::move(token));
}

void Preprocessor::include_file(const fs::path& path)
{
    vector<Token> included_tokens = scan_file(path);
    ::preprocess(included_tokens, opts_);
    tokens_.insert(tokens_.cend(), std::make_move_iterator(included_tokens.begin()), std::make_move_iterator(included_tokens.end()));
}
