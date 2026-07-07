//
// Created by jaket on 28/11/2025.
//

#include "expressions/Identifier.h"

#include "runtime/FunctionQuery.h"
#include "runtime/RuntimeUtils.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"

ExprPtr Identifier::instantiate_template_types(const TypePtr& template_type) const
{
    return std::make_unique<Identifier>(token_);
}

void Identifier::init_impl(const vector<TypePtr>& types)
{
    if (scope().has_variable(name_))
        var_ = scope().get_variable(name_);

    if (scope().has_function({types, name_, /*is_parameterless*/true}))
        var_ = RuntimeUtils::invoke_function(types, name_);

    if (var_ == nullptr)
        throw CompileError{"Variable or parameterless function not defined or ambiguous: " + name_};
}

TypePtr Identifier::type_impl() const
{
    return var_->type();
}

VarPtr Identifier::evaluate_impl() const
{
    return var_;
}
