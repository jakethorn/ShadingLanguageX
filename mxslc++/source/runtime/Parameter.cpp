//
// Created by jaket on 21/11/2025.
//

#include "Parameter.h"

#include <cassert>

#include "expressions/Expression.h"
#include "utils/template_utils.h"

Parameter::Parameter(vector<string> modifiers, Type type, Token name, const size_t index)
    : Parameter{std::move(modifiers), std::move(type), std::move(name), nullptr, index}
{

}

Parameter::Parameter(vector<string> modifiers, Type type, Token name, ExprPtr expr, const size_t index)
    : modifiers_{std::move(modifiers)},
    type_{std::move(type)},
    name_{std::move(name)},
    expr_{std::move(expr)},
    index_{index}
{

}

Parameter::Parameter(Parameter&& other) noexcept
    : modifiers_{std::move(other.modifiers_)},
    type_{std::move(other.type_)},
    name_{std::move(other.name_)},
    expr_{std::move(other.expr_)},
    index_{other.index_}
{

}

Parameter::~Parameter() = default;

Parameter Parameter::instantiate_templated_types(const Type& template_type) const
{
    Type type = type_.instantiate_template_type(template_type);
    ExprPtr expr = expr_ ? expr_->instantiate_templated_types(template_type) : nullptr;
    return Parameter{modifiers_, std::move(type), name_, std::move(expr), index_};
}

void Parameter::init() const
{
    if (has_default_value())
        expr_->init(type_);
}

ValuePtr Parameter::evaluate() const
{
    assert(expr_);
    return expr_->evaluate();
}
