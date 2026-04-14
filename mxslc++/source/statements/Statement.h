//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_STATEMENT_H
#define FENNEC_STATEMENT_H

#include "Token.h"
#include "utils/common.h"

class Runtime;

class Statement
{
public:
    explicit Statement(const Runtime& runtime, Token token) : runtime_{runtime}, token_{std::move(token)} { }
    virtual ~Statement() = default;

    const Token& token() const { return token_; }

    virtual StmtPtr instantiate_template_types(const TypeInfoPtr& template_type) const = 0;
    void execute() const;

protected:
    virtual void execute_impl() const = 0;

    const Runtime& runtime_;
    Token token_;
};

#endif //FENNEC_STATEMENT_H
