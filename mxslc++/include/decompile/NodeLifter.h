//
// Created for MXSL decompiler AST lifting.
//

#ifndef MXSLC_NODELIFTER_H
#define MXSLC_NODELIFTER_H

#include <MaterialXCore/Document.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "common.h"
#include "expressions/Expression.h"
#include "runtime/ArgumentList.h"
#include "runtime/AttributeList.h"

namespace mxslc::decompile
{
    using PortResolver = std::function<ExprPtr(const mx::PortElementPtr&)>;
    using NodeResolver = std::function<ExprPtr(const mx::NodePtr&)>;

    class NodeLifter
    {
    public:
        static string get_type_alias(const string& type_name);
        static string get_type_alias(const mx::TypedElementPtr& typed_element);

        static bool is_structural_attribute(const string& attr_name);

        static AttributeList extract_attributes(const mx::ElementPtr& element);

        static bool is_swizzle(
            const mx::NodePtr& node,
            string& out_channels,
            mx::InputPtr& out_source_input
        );

        static bool is_extract_swizzle(
            const mx::NodePtr& node,
            char& out_channel,
            mx::InputPtr& out_source_input
        );

        static bool is_constructor(
            const mx::NodePtr& node,
            string& out_constructor_name
        );

        static bool is_if_expression(const mx::NodePtr& node);

        static bool is_binary_op(
            const mx::NodePtr& node,
            string& out_dunder_name
        );

        static bool is_unary_op(
            const mx::NodePtr& node,
            string& out_dunder_name
        );

        static ExprPtr lift_node(
            const mx::NodePtr& node,
            const PortResolver& port_resolver
        );

        static ExprPtr value_to_expression(const mx::ValuePtr& value);
    };
}

#endif //MXSLC_NODELIFTER_H
