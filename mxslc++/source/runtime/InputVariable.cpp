//
// Created by jaket on 03/05/2026.
//

#include "InputVariable.h"

#include "Runtime.h"
#include "Type.h"
#include "values/ValueFactory.h"

InputVariable::InputVariable(mx::InputPtr input)
    : Variable{TokenType::Mutable, Type::of(input)},
    input_{std::move(input)}
{

}

ValuePtr InputVariable::value_impl() const
{
    return ValueFactory::copy_value_from_port(input_);
}

void InputVariable::copy_value_impl(const ValuePtr value)
{
    value->set_as_node_input(input_);
}

void InputVariable::set_node_name(const string& name) const
{

}
