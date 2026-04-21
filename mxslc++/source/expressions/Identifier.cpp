//
// Created by jaket on 28/11/2025.
//

#include "Identifier.h"

#include "runtime/Scope.h"
#include "runtime/Runtime.h"
#include "runtime/variables/IVariable.h"
#include "values/ValueFactory.h"

ExprPtr Identifier::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return std::make_unique<Identifier>(token_);
}

void Identifier::init_impl(const vector<TypeInfoPtr>& types)
{
    var_ = scope().get_variable(name());
}

TypeInfoPtr Identifier::type_impl() const
{
    return var_->type();
}

VarPtr2 Identifier::evaluate_impl() const
{
    return var_;
}
