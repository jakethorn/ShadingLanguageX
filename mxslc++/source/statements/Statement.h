//
// Created by jaket on 06/11/2025.
//

#ifndef FENNEC_STATEMENT_H
#define FENNEC_STATEMENT_H

#include "../../include/Token.h"
#include "runtime/AttributeList.h"
#include "runtime/RuntimeAccessor.h"
#include "utils/common.h"

class Statement : protected RuntimeAccessor
{
public:
    explicit Statement(Token token) : token_{std::move(token)} { }
    ~Statement() override = default;

    const Token& token() const { return token_; }

    virtual void set_attributes(AttributeList attrs) { }

    virtual StmtPtr instantiate_template_types(const TypePtr& template_type) const = 0;
    virtual void init() { }
    void execute();

protected:
    virtual void execute_impl() const = 0;

    Token token_;
    bool is_initialized_ = false;
};

#endif //FENNEC_STATEMENT_H
