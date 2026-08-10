//
// Created by jaket on 07/11/2025.
//

#ifndef FENNEC_EXPRESSION_H
#define FENNEC_EXPRESSION_H

#include "common.h"
#include "Token.h"
#include "runtime/AttributeList.h"
#include "runtime/ModifierList.h"
#include "runtime/utils/monomorphize.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::expressions
{
    using runtime_utils::Monomorphizable;
    using runtime_utils::RuntimeAware;

    class Expression : public Monomorphizable<ExprPtr>, public Stringable, protected RuntimeAware
    {
    public:
        Expression() = default;
        explicit Expression(Token token);

        ~Expression() override = default;

        const Token& token() const { return token_; }

        void set_modifiers(ModifierList mods) { mods_ = std::move(mods); }
        void set_attributes(AttributeList attrs) { attrs_ = std::move(attrs); }

        ExprPtr monomorphize(const TypePtr& template_type) const override = 0;

        void init();
        void init(const TypePtr& target_type);
        void init(const vector<TypePtr>& target_types);
        bool try_init(const TypePtr& target_type);
        bool try_init(const vector<TypePtr>& target_types);
        void update();
        void reset();

        bool is_initialized() const { return is_initialized_; }
        bool is_comptime() const { return mods_.contains(TokenType::Comptime); }

        TypePtr type() const;
        VarPtr evaluate() const;
        void assign(const VarPtr& value) const;

        bool has_error() const { return not error_message_.empty(); }
        const string& error_message() const { return error_message_; }

        string to_string() const override = 0;

    protected:
        virtual void init_subexpressions(const vector<TypePtr>& types) { }
        virtual void init_impl(const vector<TypePtr>& types) { }
        virtual TypePtr type_impl() const = 0;
        virtual VarPtr evaluate_impl() const = 0;

        Token token_;

        TypePtr target_type_;
        string error_message_;
        bool is_initialized_{false};

        ModifierList mods_;
        AttributeList attrs_;

    private:
        bool update_target_type(const TypePtr& type, const vector<TypePtr>& target_types);
    };
}

#endif //FENNEC_EXPRESSION_H
