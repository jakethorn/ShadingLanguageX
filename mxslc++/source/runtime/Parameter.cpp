//
// Created by jaket on 21/11/2025.
//

#include "runtime/Parameter.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "errors/CompileError.h"
#include "runtime/variables/Variable.h"

namespace mxslc::runtime
{
    Parameter::Parameter(AttributeList attrs, ModifierList mods, TypePtr type, string name, ExprPtr expr, const size_t index)
        : attrs_{std::move(attrs)},
        mods_{std::move(mods)},
        type_{std::move(type)},
        name_{std::move(name)},
        expr_{std::move(expr)},
        index_{index}
    {
        mods_.validate(TokenType::Const, TokenType::Comptime, TokenType::Mutable, TokenType::Ref, TokenType::Out);

        if (mods_.contains(TokenType::Ref) or mods_.contains(TokenType::Out))
            mods_.add(TokenType::Mutable);

        if (is_const() and is_mutable())
            throw CompileError{"Parameters cannot be both const and mutable (ref and out parameters are mutable by default)"};

        if (mods_.contains(TokenType::Ref) and mods_.contains(TokenType::Out))
            throw CompileError{"Parameters cannot be both ref and out"};

    }

    Parameter Parameter::monomorphize(const TypePtr& template_type) const
    {
        auto [type, expr] = runtime_utils::monomorphize_all(template_type, type_, expr_);
        return Parameter{attrs_, mods_, std::move(type), name_, std::move(expr), index_};
    }

    void Parameter::init()
    {
        type_ = scope().resolve_type(type_);

        if (has_default_value())
        {
            expr_->init(type_);
            type_ = expr_->type();
        }
        else if (type_->is_auto())
        {
            throw CompileError{"Auto parameter '" + name_ + "' must have an initializer"};
        }
    }

    VarPtr Parameter::evaluate() const
    {
        VarPtr value = expr_->evaluate();
        if (not initial_value_)
            initial_value_ = value->copy();
        return value;
    }

    string Parameter::to_string() const
    {
        return to_string(true);
    }

    string Parameter::to_string(const bool show_null_default) const
    {
        string attrs_string = attrs_.to_string(" ");
        if (not attrs_string.empty())
            attrs_string += ' ';

        // out and ref parameters are implicitly mutable
        string mods_string = (is_out() ? mods_.without(TokenType::Mutable) : mods_).to_string();
        if (not mods_string.empty())
            mods_string += ' ';

        string default_value_string = has_default_value() ? " = " + expr_->to_string() : "";
        if (not show_null_default and default_value_string == " = null")
            default_value_string.clear();

        return attrs_string + mods_string + type_->to_string() + ' ' + name_ + default_value_string;
    }
}
