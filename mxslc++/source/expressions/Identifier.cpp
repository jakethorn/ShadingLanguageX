//
// Created by jaket on 28/11/2025.
//

#include "Identifier.h"
#include "runtime/Scope.h"
#include "runtime/Runtime.h"
#include "runtime/Variable.h"

ExprPtr Identifier::instantiate_templated_types(const Type& template_type) const
{
    return std::make_unique<Identifier>(runtime_, token_);
}

void Identifier::init_impl(const vector<Type>& types)
{
    const Scope& scope = runtime_.scope();
    var_ = &scope.get_variable(token_);
}

const Type& Identifier::type_impl() const
{
    return var_->type();
}

ValuePtr Identifier::evaluate_impl() const
{
    return var_->value();
}
