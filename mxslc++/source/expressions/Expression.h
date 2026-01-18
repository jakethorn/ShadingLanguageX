//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_EXPRESSION_H
#define FENNEC_EXPRESSION_H

#include "utils/common.h"
#include "runtime/Type.h"
#include "Token.h"

class Runtime;

class Expression
{
public:
    explicit Expression(const Runtime& runtime);
    Expression(const Runtime& runtime, Token token);
    virtual ~Expression() = default;

    [[nodiscard]] const Token& token() const { return token_; }

    void init();
    void init(const TypeInfoPtr& type);
    void init(const string& type_name);
    void init(const vector<TypeInfoPtr>& types);
    [[nodiscard]] bool try_init(const vector<TypeInfoPtr>& types);

    [[nodiscard]] bool is_initialized() const { return is_initialized_; }
    [[nodiscard]] TypeInfoPtr type() const;
    [[nodiscard]] ValuePtr evaluate() const;

    [[nodiscard]] virtual ExprPtr instantiate_template_types(const TypeInfoPtr& template_type) const = 0;

protected:
    virtual void init_subexpressions(const vector<TypeInfoPtr>& types) { }
    virtual void init_impl(const vector<TypeInfoPtr>& types) { }
    [[nodiscard]] virtual TypeInfoPtr type_impl() const = 0;
    [[nodiscard]] virtual ValuePtr evaluate_impl() const = 0;

    const Runtime& runtime_;
    Token token_;
    bool is_initialized_ = false;
};

#endif //FENNEC_EXPRESSION_H
