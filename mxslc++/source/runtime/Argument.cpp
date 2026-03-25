//
// Created by jaket on 21/11/2025.
//

#include "Argument.h"
#include "expressions/Expression.h"

Argument::Argument(ExprPtr expr, const size_t index) : Argument{std::nullopt, std::move(expr), index} { }
Argument::Argument(optional<string> name, ExprPtr expr, const size_t index) : name_{std::move(name)}, expr_{std::move(expr)}, index_{index} { }
Argument::Argument(Argument&& other) noexcept : name_{std::move(other.name_)}, expr_{std::move(other.expr_)}, index_{other.index_} { }

Argument::~Argument() = default;

Argument Argument::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return Argument{name_, expr_->instantiate_template_types(template_type), index_};
}

void Argument::init(const vector<TypeInfoPtr>& types) const
{
    expr_->init(types);
}

bool Argument::try_init(const vector<TypeInfoPtr>& types) const
{
    return expr_->try_init(types);
}

bool Argument::is_initialized() const
{
    return expr_->is_initialized();
}

TypeInfoPtr Argument::type() const
{
    return expr_->type();
}

ValuePtr Argument::evaluate() const
{
    return expr_->evaluate();
}
