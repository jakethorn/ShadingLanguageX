//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_EXPRESSION_H
#define FENNEC_EXPRESSION_H

#include "utils/common.h"
#include "Token.h"
#include "runtime/AttributeList.h"

class MtlXSerializer;

class Expression
{
public:
    Expression() = default;
    explicit Expression(Token token);
    virtual ~Expression() = default;

    const Token& token() const { return token_; }

    void set_attributes(AttributeList attrs) { attrs_ = std::move(attrs); }

    virtual ExprPtr instantiate_template_types(const TypePtr& template_type) const = 0;

    void init();
    void init(const TypePtr& type);
    void init(const string& type_name);
    void init(const vector<TypePtr>& types);
    bool try_init(const vector<TypePtr>& types);

    bool is_initialized() const { return is_initialized_; }
    TypePtr type() const;

    VarPtr evaluate() const;

protected:
    virtual void init_subexpressions(const vector<TypePtr>& types) { }
    virtual void init_impl(const vector<TypePtr>& types) { }
    virtual TypePtr type_impl() const = 0;
    virtual VarPtr evaluate_impl() const = 0;

    static Scope& scope();
    static MtlXSerializer& serializer();

    Token token_;
    bool is_initialized_ = false;
    TypePtr assigned_type_ = nullptr;

    AttributeList attrs_;
};

#endif //FENNEC_EXPRESSION_H
