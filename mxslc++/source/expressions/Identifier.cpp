//
// Created by jaket on 28/11/2025.
//

#include "Identifier.h"

#include <cassert>

#include "runtime/Scope.h"
#include "runtime/Runtime.h"
#include "runtime/TypeInfo.h"
#include "runtime/Variable.h"
#include "values/Value.h"
#include "values/ValueFactory.h"

ExprPtr Identifier::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return std::make_unique<Identifier>(runtime_, token_);
}

void Identifier::init_impl(const vector<TypeInfoPtr>& types)
{
    var_ = runtime_.scope().get_variable(token_);
}

TypeInfoPtr Identifier::type_impl() const
{
    return var_->type();
}

ValuePtr Identifier::evaluate_impl() const
{
    // check if value is in the current node_graph
    // if it is:
        // all good
    // else:
        // add input to the node_def with the correct type
        // instead return an interface_value
        // save variable and use it in mtlx_serializer::write_node, passing its value to the node with the correct input from the node_def

    // do the appropriate similar logic for assign
    // and also in dot_expression and indexing_expression

    return ValueFactory::cast_value(var_->value(), type());
}

void Identifier::assign(const ValuePtr& value)
{
    var_->set_value(value);
}
