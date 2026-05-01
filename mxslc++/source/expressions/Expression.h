//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_EXPRESSION_H
#define FENNEC_EXPRESSION_H

#include "utils/common.h"
#include "Token.h"

class MtlXSerializer;

class Expression
{
public:
    Expression() = default;
    explicit Expression(Token token);

    virtual ~Expression() = default;

    virtual ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const = 0;

    void init();
    void init(const TypeInfoPtr& type);
    void init(const string& type_name);
    void init(const vector<TypeInfoPtr>& types);
    bool try_init(const vector<TypeInfoPtr>& types);

    const Token& token() const { return token_; }
    bool is_initialized() const { return is_initialized_; }
    TypeInfoPtr type() const;

    VarPtr evaluate() const;

protected:
    virtual void init_subexpressions(const vector<TypeInfoPtr>& types) { }
    virtual void init_impl(const vector<TypeInfoPtr>& types) { }
    virtual TypeInfoPtr type_impl() const = 0;
    virtual VarPtr evaluate_impl() const = 0;

    static Scope& scope();
    static MtlXSerializer& serializer();

    Token token_;
    bool is_initialized_ = false;
    TypeInfoPtr assigned_type_ = nullptr;
};

#endif //FENNEC_EXPRESSION_H
