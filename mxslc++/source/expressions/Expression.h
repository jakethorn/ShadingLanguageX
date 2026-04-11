//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_EXPRESSION_H
#define FENNEC_EXPRESSION_H

#include "utils/common.h"
#include "Token.h"

class Runtime;

class Expression
{
public:
    explicit Expression(const Runtime& runtime);
    Expression(const Runtime& runtime, Token token);
    virtual ~Expression() = default;

    const Token& token() const { return token_; }

    void init();
    void init(const TypeInfoPtr& type);
    void init(const string& type_name);
    void init(const vector<TypeInfoPtr>& types);
    bool try_init(const vector<TypeInfoPtr>& types);

    bool is_initialized() const { return is_initialized_; }
    TypeInfoPtr type() const;
    virtual VarPtr variable() const { return nullptr; }

    virtual ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const = 0;

    ValuePtr evaluate() const;
    void assign(const ValuePtr& value);

protected:
    virtual void init_subexpressions(const vector<TypeInfoPtr>& types) { }
    virtual void init_impl(const vector<TypeInfoPtr>& types) { }
    virtual TypeInfoPtr type_impl() const = 0;
    virtual ValuePtr evaluate_impl() const = 0;
    virtual void assign_impl(const ValuePtr& value);

    const Runtime& runtime_;
    Token token_;
    TypeInfoPtr assigned_type_ = nullptr;
    bool is_initialized_ = false;
};

#endif //FENNEC_EXPRESSION_H
