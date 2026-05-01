//
// Created by jaket on 21/11/2025.
//

#include "Argument.h"

#include "statements/Statement.h"
#include "CompileError.h"
#include "Parameter.h"
#include "expressions/Expression.h"
#include "expressions/VariableDefinitionExpression.h"

Argument::Argument(ModifierList mods, string name, ExprPtr expr, const size_t index)
    : mods_{std::move(mods)}, name_{std::move(name)}, expr_{std::move(expr)}, index_{index}
{
    mods_.validate(TokenType::Ref, TokenType::Out);
    if (mods_.contains(TokenType::Ref) and mods_.contains(TokenType::Out))
        throw CompileError{"An argument cannot be both ref and out"s};
}

Argument::Argument(ModifierList mods, ExprPtr expr, const size_t index)
    : Argument{std::move(mods), ""s, std::move(expr), index} { }

Argument::Argument(string name, ExprPtr expr, const size_t index)
    : Argument{{}, std::move(name), std::move(expr), index} { }

Argument::Argument(ExprPtr expr, const size_t index)
    : Argument{{}, ""s, std::move(expr), index} { }

Argument::Argument(Argument&& other) noexcept
    : mods_{std::move(other.mods_)}, name_{std::move(other.name_)}, expr_{std::move(other.expr_)}, index_{other.index_} { }

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

VarPtr Argument::evaluate() const
{
    return expr_->evaluate();
}

void Argument::validate(const Parameter& param) const
{
    if (mods_.contains(TokenType::Ref) and not param.modifiers().contains(TokenType::Ref))
        throw CompileError{"Ref argument is being passed to a non-ref parameter"s};

    if (mods_.contains(TokenType::Out) and not param.modifiers().contains(TokenType::Out))
        throw CompileError{"Out argument is being passed to a non-out parameter"s};

    if (std::dynamic_pointer_cast<VariableDefinitionExpression>(expr_) and not param.modifiers().contains(TokenType::Out))
        throw CompileError{"Variable definition expressions can only be passed to out parameter"s};
}
