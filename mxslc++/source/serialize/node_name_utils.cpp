//
// Created by jaket on 30/09/2026.
//

#include "serialize/node_name_utils.h"

#include <algorithm>

#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "serialize/values/interface.h"
#include "serialize/values/NodeOutputValue.h"
#include "serialize/name_prefix_utils.h"
#include "serialize/values/NodeValue.h"

namespace mxslc::serialize
{
    namespace
    {
        // the name of a field in the names of nodes, which is its index if the field has no name
        string get_field_name(const VarPtr& var, const size_t index)
        {
            const Field& field = var->type()->field(index);
            return field.has_name() ? field.name() : std::to_string(index);
        }

        // e.g., x for a variable x, and s__lo for the field lo of a struct s
        string get_node_name(const VarPtr& var)
        {
            if (not var->has_parent())
                return var->name();

            // fields are named after their index, e.g., s__0, see Variable::set_name, unless they are also added to a
            // scope by their own name, e.g., the fields of `this` in a method
            const VarPtr parent = var->parent();
            const vector<VarPtr>& fields = parent->children();
            const size_t index = std::find(fields.begin(), fields.end(), var) - fields.begin();
            if (var->name() != with_prefix(parent->name(), index))
                return var->name();
            return parent->name() + "__" + get_field_name(parent, index);
        }

        void name_assigned_nodes_of_fields(const VarPtr& var, const VarPtr& value)
        {
            if (value->has_value())
            {
                // values from outside of the graph, e.g., the nonlocal variables that a function reads, keep their names
                if (not value->is_temporary() and not value->is_local())
                    return;
                if (const NodeValuePtr node_value = cast_value<NodeValue>(value->raw_value()))
                    node_value->set_assigned_node_name(get_node_name(var));
                return;
            }

            for (size_t i = 0; i < var->child_count() and i < value->child_count(); ++i)
                name_assigned_nodes_of_fields(var->child(i), value->child(i));
        }
    }

    void name_nodes(const VarPtr& var)
    {
        for (const VarPtr& field : var->children())
            name_nodes(field);

        const string node_name = get_node_name(var);
        if (const NodeValuePtr value = cast_value<NodeValue>(var->raw_value()))
        {
            value->set_node_name(node_name);
            return;
        }

        // a struct whose fields are the outputs of a single node is named after the struct, e.g., the node of a
        // function that returns a struct
        mx::NodePtr node;
        for (const VarPtr& field : var->children())
        {
            if (const NodeOutputValuePtr output = cast_value<NodeOutputValue>(field->raw_value()))
            {
                if (node and output->node() != node)
                    return;
                node = output->node();
            }
        }

        for (const VarPtr& field : var->children())
        {
            if (const NodeOutputValuePtr output = cast_value<NodeOutputValue>(field->raw_value()))
                output->set_node_name(node_name);
        }
    }

    void name_assigned_nodes(const VarPtr& var, const VarPtr& value)
    {
        // the value of the definition of a variable is named by name_nodes
        if (not var->is_initialized() or var->is_temporary())
            return;

        // the values assigned to the variables of inline functions belong to the code that calls the function
        if (var->defining_scope()->is_inside_inline_function())
            return;

        name_assigned_nodes_of_fields(var, value);
    }

    void disable_node_naming(const VarPtr& var)
    {
        for (const VarPtr& field : var->children())
            disable_node_naming(field);

        const ValuePtr value = var->raw_value();
        if (const NodeValuePtr node_value = cast_value<NodeValue>(value))
            node_value->disable_node_naming();
        else if (const NodeOutputValuePtr output_value = cast_value<NodeOutputValue>(value))
            output_value->disable_node_naming();
    }
}
