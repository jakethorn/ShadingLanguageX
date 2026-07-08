//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_EXPRESSION_H
#define FENNEC_EXPRESSION_H

#include "common.h"
#include "Token.h"
#include "runtime/AttributeList.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::expressions
{
    class Expression : protected runtime_utils::RuntimeAware
    {
    public:
        Expression() = default;
        explicit Expression(Token token);

        ~Expression() override = default;

        const Token& token() const { return token_; }

        void set_attributes(AttributeList attrs) { attrs_ = std::move(attrs); }

        virtual ExprPtr instantiate_template_types(const TypePtr& template_type) const = 0;

        void set_subexpression_type(TypePtr type) { subexpr_type_ = std::move(type); }

        void init();
        void init(const TypePtr& type);
        void init(const vector<TypePtr>& types);
        bool try_init(const vector<TypePtr>& types);
        void update();

        bool is_initialized() const { return is_initialized_; }
        void reset();
        const string& error_message() const { return error_message_; }

        TypePtr type() const;

        VarPtr evaluate() const;

    protected:
        virtual void init_subexpressions(const vector<TypePtr>& types) { }
        virtual void init_impl(const vector<TypePtr>& types) { }
        virtual TypePtr type_impl() const = 0;
        virtual VarPtr evaluate_impl() const = 0;

        Token token_;

        bool is_initialized_{false};
        TypePtr subexpr_type_;
        TypePtr target_type_;
        string error_message_;

        AttributeList attrs_;
    };
}

#endif //FENNEC_EXPRESSION_H
