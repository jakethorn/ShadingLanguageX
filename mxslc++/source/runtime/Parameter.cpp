//
// Created by jaket on 21/11/2025.
//

#include "Parameter.h"

#include <cassert>

#include "CompileError.h"
#include "expressions/Expression.h"
#include "utils/template_utils.h"

Parameter::Parameter(vector<string> mods, Type type, Token name, const size_t index)
    : Parameter{std::move(mods), std::move(type), std::move(name), nullptr, index}
{

}

Parameter::Parameter(vector<string> mods, Type type, Token name, ExprPtr expr, const size_t index)
    : mods_{std::move(mods)},
    type_{std::move(type)},
    name_{std::move(name)},
    expr_{std::move(expr)},
    index_{index}
{
    static const vector valid_mods{"const"s, "mutable"s, "out"s};
    for (const string& mod : mods_)
        if (not contains(valid_mods, mod))
            throw CompileError{name_, "'" + mod + "' is not a valid parameter modifier"};
}

Parameter::Parameter(Parameter&& other) noexcept
    : mods_{std::move(other.mods_)},
    type_{std::move(other.type_)},
    name_{std::move(other.name_)},
    expr_{std::move(other.expr_)},
    index_{other.index_}
{

}

Parameter::~Parameter() = default;

Parameter Parameter::instantiate_template_types(const Type& template_type) const
{
    Type type = type_.instantiate_template_types(template_type);
    ExprPtr expr = expr_ ? expr_->instantiate_template_types(template_type) : nullptr;
    return Parameter{mods_, std::move(type), name_, std::move(expr), index_};
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
