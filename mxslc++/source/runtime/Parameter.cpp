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
        mods_.validate(TokenType::Const, TokenType::Mutable, TokenType::Ref, TokenType::Out);

        if (mods_.contains(TokenType::Ref) or mods_.contains(TokenType::Out))
            mods_.add(TokenType::Mutable);

        if (is_const() and is_mutable())
            throw CompileError{"Parameters cannot be both const and mutable (ref and out parameters are mutable by default)"};

        if (mods_.contains(TokenType::Ref) and mods_.contains(TokenType::Out))
            throw CompileError{"Parameters cannot be both ref and out"};

    }

    Parameter::Parameter(Parameter&& other) noexcept
        : attrs_{std::move(other.attrs_)},
        mods_{std::move(other.mods_)},
        type_{std::move(other.type_)},
        name_{std::move(other.name_)},
        expr_{std::move(other.expr_)},
        index_{other.index_}
    {

    }

    Parameter::~Parameter() = default;

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

    TypePtr Parameter::type() const
    {
        return type_;
    }

    VarPtr Parameter::evaluate() const
    {
        return expr_->evaluate();
    }

    string Parameter::str() const
    {
        string result;
        result += mods_.str();
        result += type_->str();
        result += " " + name_;
        return result;
    }
}
