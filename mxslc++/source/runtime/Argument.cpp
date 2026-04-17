//
// Created by jaket on 21/11/2025.
//

#include "Argument.h"

#include "CompileError.h"
#include "Parameter.h"
#include "expressions/Expression.h"

Argument::Argument(const bool is_out, string name, ExprPtr expr, const size_t index)
    : is_out_{is_out}, name_{std::move(name)}, expr_{std::move(expr)}, index_{index} { }

Argument::Argument(string name, ExprPtr expr, const size_t index)
    : Argument{false, std::move(name), std::move(expr), index} { }

Argument::Argument(const bool is_out, ExprPtr expr, const size_t index)
    : Argument{is_out, ""s, std::move(expr), index} { }

Argument::Argument(ExprPtr expr, const size_t index)
    : Argument{false, ""s, std::move(expr), index} { }

Argument::Argument(Argument&& other) noexcept
    : is_out_{other.is_out_}, name_{std::move(other.name_)}, expr_{std::move(other.expr_)}, index_{other.index_} { }

Argument::~Argument() = default;

Argument Argument::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return Argument{name_, expr_->instantiate_template_types(template_type), index_};
}

void Argument::init(const TypeInfoPtr& type) const
{
    expr_->init(type);
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

VarPtr2 Argument::evaluate() const
{
    return expr_->evaluate();
}

void Argument::validate(const Parameter& param) const
{
    if (is_out() and not param.is_out())
    {
        throw CompileError{"Out argument is being passed to a non-out parameter"s};
    }
}
