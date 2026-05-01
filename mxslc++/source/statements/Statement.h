//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_STATEMENT_H
#define FENNEC_STATEMENT_H

#include "Token.h"
#include "utils/common.h"

class MtlXSerializer;
class Runtime;

class Statement
{
public:
    explicit Statement(Token token) : token_{std::move(token)} { }
    virtual ~Statement() = default;

    const Token& token() const { return token_; }

    virtual StmtPtr instantiate_template_types(const TypePtr& template_type) const = 0;
    void execute() const;

protected:
    virtual void execute_impl() const = 0;

    static Runtime& runtime();

    Token token_;
};

#endif //FENNEC_STATEMENT_H
