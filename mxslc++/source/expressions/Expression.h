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

    void init();
    void init(const Type& type);
    void init(const vector<Type>& types);
    [[nodiscard]] bool try_init(const vector<Type>& types);

    [[nodiscard]] bool is_initialized() const { return is_initialized_; }
    [[nodiscard]] const Type& type() const;
    [[nodiscard]] ValuePtr evaluate() const;

    [[nodiscard]] virtual ExprPtr instantiate_template_types(const Type& template_type) const = 0;
    [[nodiscard]] virtual ExprPtr copy() const { return instantiate_template_types(Type{"T"s}); }

protected:
    virtual void init_child_expressions(const vector<Type>& types) { }
    virtual void init_impl(const vector<Type>& types) { }
    [[nodiscard]] virtual const Type& type_impl() const = 0;
    [[nodiscard]] virtual ValuePtr evaluate_impl() const = 0;

    const Runtime& runtime_;
    Token token_;
    bool is_initialized_ = false;
};

#endif //FENNEC_EXPRESSION_H
