//
// Created by jaket on 04/05/2026.
//

#include "expressions/accessors/PortAccessor.h"

#include <cassert>

#include "errors/CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "runtime/variables/PortVariable.h"
#include "serialize/values/interface.h"
#include "serialize/values/NodeValue.h"
#include "utils/mtlx_utils.h"

namespace mxslc::expressions
{
    PortAccessor::PortAccessor(VarPtr node_var, string input_name) : node_var_{std::move(node_var)}, input_name_{std::move(input_name)}
    {
        assert(node_var_->has_value());

        const ValuePtr value = node_var_->raw_value();
        const NodeValuePtr node_value = cast_value<NodeValue>(value);

        if (node_value == nullptr)
            throw CompileError{"Variable of type '" + node_var_->type()->to_string() + "' does not have a port or valid swizzle with the name: " + input_name_};

        const mx::NodePtr node = node_value->node();
        const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node, runtime().materialx_library());

        const mx::InputPtr input = node_def->getActiveInput(input_name_);
        if (input == nullptr)
            throw CompileError{"Variable of type '" + node_var_->type()->to_string() + "' does not have a port or valid swizzle with the name: " + input_name_};

        input_ = mtlx_utils::add_or_get_input(node, input->getType(), input_name_);
    }

    TypePtr PortAccessor::type() const
    {
        return Type::of(input_);
    }

    VarPtr PortAccessor::evaluate() const
    {
        string input_var_name = node_var_->name() + "__" + input_name_;
        if (scope().has_variable(input_var_name))
            return scope().get_variable(input_var_name);
        VarPtr input_var = std::make_shared<PortVariable>(input_);
        node_var_->defining_scope()->add_variable(std::move(input_var_name), input_var);
        return input_var;
    }
}
