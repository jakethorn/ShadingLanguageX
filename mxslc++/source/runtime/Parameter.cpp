//
// Created by jaket on 21/11/2025.
//

#include "Parameter.h"
#include "expressions/Expression.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"

Parameter::Parameter(const Runtime& runtime, ModifierList mods, Token type, Token name, const size_t index)
    : Parameter{runtime, std::move(mods), std::move(type), std::move(name), nullptr, index}
{

}

Parameter::Parameter(const Runtime& runtime, ModifierList mods, Token type, Token name, ExprPtr expr, const size_t index)
    : runtime_{runtime},
    mods_{std::move(mods)},
    type_{std::move(type)},
    name_{std::move(name)},
    expr_{std::move(expr)},
    index_{index}
{
    mods_.validate("const"s, "mutable"s, "out"s);
}

Parameter::Parameter(Parameter&& other) noexcept
    : runtime_{other.runtime_},
    mods_{std::move(other.mods_)},
    type_{std::move(other.type_)},
    name_{std::move(other.name_)},
    expr_{std::move(other.expr_)},
    index_{other.index_}
{

}

Parameter::~Parameter() = default;

Parameter Parameter::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    Token type = type_.instantiate_template_types(template_type);
    ExprPtr expr = expr_ ? expr_->instantiate_template_types(template_type) : nullptr;
    return Parameter{runtime_, mods_, std::move(type), name_, std::move(expr), index_};
}

void Parameter::init() const
{
    if (has_default_value())
        expr_->init(type());
}

TypeInfoPtr Parameter::type() const
{
    return runtime_.scope().get_type(type_);
}

ValuePtr Parameter::evaluate() const
{
    return expr_->evaluate();
}
