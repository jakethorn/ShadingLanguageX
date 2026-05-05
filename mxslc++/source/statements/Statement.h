//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_STATEMENT_H
#define FENNEC_STATEMENT_H

#include "Token.h"
#include "runtime/AttributeList.h"
#include "utils/common.h"

class Scope;
class MtlXSerializer;

class Statement
{
public:
    explicit Statement(Token token) : token_{std::move(token)} { }
    virtual ~Statement() = default;

    const Token& token() const { return token_; }

    virtual void set_attributes(AttributeList attrs) { }

    virtual StmtPtr instantiate_template_types(const TypePtr& template_type) const = 0;
    void execute() const;

protected:
    virtual void execute_impl() const = 0;

    static Scope& scope();
    static MtlXSerializer& serializer();

    Token token_;
};

#endif //FENNEC_STATEMENT_H
