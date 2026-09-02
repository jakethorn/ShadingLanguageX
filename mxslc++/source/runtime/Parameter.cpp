//
// Created by jaket on 21/11/2025.
//

#include "runtime/Parameter.h"

#include "runtime/Scope.h"
#include "expressions/Expression.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "errors/CompileError.h"

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
            expr_->init(type());
    }

    VarPtr Parameter::evaluate() const
    {
        return expr_->evaluate();
    }

    string Parameter::to_string() const
    {
        string mods_string = mods_.to_string();
        if (not mods_string.empty())
            mods_string += " ";

        string default_value_string = has_default_value() ? expr_->to_string() : "";
        if (default_value_string.empty() or default_value_string == "null")
            default_value_string = "";
        else
            default_value_string = " = " + default_value_string;

        return mods_string + type_->to_string() + " " + name_ + default_value_string;
    }
}
