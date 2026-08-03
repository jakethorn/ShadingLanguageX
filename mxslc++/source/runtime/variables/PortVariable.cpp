//
// Created by jaket on 03/05/2026.
//

#include "runtime/variables/PortVariable.h"

#include "runtime/Runtime.h"
#include "runtime/Type.h"
#include "serialize/serializer_utils.h"
#include "serialize/values/Value.h"

namespace mxslc::runtime
{
    PortVariable::PortVariable(mx::InputPtr input)
        : Variable{TokenType::Mutable, Type::of(input)},
        input_{std::move(input)}
    {

    }

    ValuePtr PortVariable::value_impl() const
    {
        return serialize_utils::copy_value_from_port(input_);
    }

    void PortVariable::copy_value_impl(const ValuePtr value)
    {
        value->set_as_node_input(input_);
    }

    void PortVariable::set_node_name(const string& name) const
    {

    }
}
