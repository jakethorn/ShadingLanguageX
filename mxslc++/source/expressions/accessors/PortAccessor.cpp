//
// Created by jaket on 04/05/2026.
//

#include "PortAccessor.h"

#include "CompileError.h"
#include "expressions/Expression.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/InputVariable.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Variable.h"
#include "values/NodeValue.h"

namespace
{
    mx::NodeDefPtr find_node_def(const mx::NodePtr& node)
    {
        mx::NodeDefPtr node_def = node->getNodeDef();
        if (node_def != nullptr)
            return node_def;

        const mx::DocumentPtr mtlx_lib = Runtime::get().materialx_library();
        const mx::NodePtr copy = mtlx_lib->addNode(node->getCategory());
        copy->copyContentFrom(node);

        node_def = copy->getNodeDef();
        if (node_def != nullptr)
        {
            mtlx_lib->removeNode(copy->getName());
            return node_def;
        }

        throw CompileError{"Cannot find NodeDef for " + node->getCategory()};
    }
}

PortAccessor::PortAccessor(VarPtr node_var, string input_name) : node_var_{std::move(node_var)}, input_name_{std::move(input_name)}
{
    if (not node_var_->has_value())
        throw CompileError{"The port access (dot) operator cannot be used on values with custom types"s};

    const ValuePtr value = node_var_->raw_value();
    const shared_ptr<NodeValue> node_value = std::dynamic_pointer_cast<NodeValue>(value);

    if (node_value == nullptr)
        throw CompileError{"The port access (dot) operator can only be used on values representing MaterialX nodes"s};

    const mx::NodePtr node = node_value->node();
    const mx::NodeDefPtr node_def = find_node_def(node);

    const mx::InputPtr input = node_def->getActiveInput(input_name_);
    if (input == nullptr)
        throw CompileError{"No input named '" + input_name_ + "' found in NodeDef " + node_def->getName()};

    input_ = add_or_get_input(node, input->getType(), input_name_);
}

TypePtr PortAccessor::type() const
{
    return Runtime::get().scope().get_type(input_->getType());
}

VarPtr PortAccessor::evaluate() const
{
    string input_var_name = node_var_->name() + "__" + input_name_;
    if (Runtime::get().scope().has_variable(input_var_name))
        return Runtime::get().scope().get_variable(input_var_name);
    VarPtr input_var = std::make_shared<InputVariable>(input_);
    node_var_->defining_scope().add_variable(std::move(input_var_name), input_var);
    return input_var;
}
