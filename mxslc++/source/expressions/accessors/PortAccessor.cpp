//
// Created by jaket on 04/05/2026.
//

#include "expressions/accessors/PortAccessor.h"

#include "expressions/Expression.h"
#include "../../../include/utils/mtlx_utils.h"
#include "runtime/InputVariable.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "serialize/values/NodeValue.h"
#include "errors/CompileError.h"
#include "serialize/values/interface.h"

namespace mxslc::expressions
{
    PortAccessor::PortAccessor(VarPtr node_var, string input_name) : node_var_{std::move(node_var)}, input_name_{std::move(input_name)}
    {
        if (not node_var_->has_value())
            throw CompileError{"The port access (dot) operator cannot be used on values with custom types"};

        const ValuePtr value = node_var_->raw_value();
        const NodeValuePtr node_value = cast_value<NodeValue>(value);

        if (node_value == nullptr)
            throw CompileError{"The port access (dot) operator can only be used on values representing MaterialX nodes"};

        const mx::NodePtr node = node_value->node();
        const mx::NodeDefPtr node_def = mtlx_utils::get_node_def(node, runtime().materialx_library());

        const mx::InputPtr input = node_def->getActiveInput(input_name_);
        if (input == nullptr)
            throw CompileError{"No input named '" + input_name_ + "' found in NodeDef " + node_def->getName()};

        input_ = mtlx_utils::add_or_get_input(node, input->getType(), input_name_);
    }

    TypePtr PortAccessor::type() const
    {
        return scope().get_type(input_->getType());
    }

    VarPtr PortAccessor::evaluate() const
    {
        string input_var_name = node_var_->name() + "__" + input_name_;
        if (scope().has_variable(input_var_name))
            return scope().get_variable(input_var_name);
        VarPtr input_var = std::make_shared<InputVariable>(input_);
        node_var_->defining_scope().add_variable(std::move(input_var_name), input_var);
        return input_var;
    }
}
