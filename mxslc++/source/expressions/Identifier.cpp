//
// Created by jaket on 28/11/2025.
//

#include "Identifier.h"
#include "runtime/Scope.h"
#include "runtime/Runtime.h"
#include "runtime/Variable.h"

ExprPtr Identifier::instantiate_template_types(const Type& template_type) const
{
    return std::make_unique<Identifier>(runtime_, token_);
}

void Identifier::init_impl(const vector<Type>& types)
{
    const Scope& scope = runtime_.scope();
    var_ = scope.get_variable(token_);
}

const Type& Identifier::type_impl() const
{
    return var_.type();
}

ValuePtr Identifier::evaluate_impl() const
{
    return var_.value();
}

/*

Identifier::init_impl is called and a raw pointer to the Variable object is stored.
Afterwards, Runtime::enter_scope is called which moves the parent scope into a new one.
This moves the position of the Variable object in memory, but the pointer isn't updated and now points to an invalid memory address.
Finally, Identifier::evaluate_impl is called with the invalid pointer.

The same thing is probably happening to FunctionCall and Function.

The solution is either:
  copy the Variable object instead of getting a pointer to it
OR
  use VariablePtr

As a first test, copy, if it works, then you can change to VariablePtr.

*/