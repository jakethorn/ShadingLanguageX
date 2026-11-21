//
// Created by jaket on 17/11/2026.
//

#include "expressions/NodeExpression.h"

#include "expressions/interface.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "serialize/Serializer.h"

namespace mxslc::expressions
{
    NodeExpression::NodeExpression(string category, TypePtr template_type, ArgumentList args, Token token)
        : Expression{std::move(token)},
        category_{std::move(category)},
        template_type_{std::move(template_type)},
        args_{std::move(args)}
    {

    }

    ExprPtr NodeExpression::monomorphize(const TypePtr& template_type) const
    {
        return create_expression<NodeExpression>(
            category_,
            runtime_utils::monomorphize(template_type_, template_type),
            runtime_utils::monomorphize(args_, template_type),
            token_
        );
    }

    void NodeExpression::init_impl(const vector<TypePtr>& types)
    {
        template_type_ = type_utils::resolve(template_type_);
        node_type_ = template_type_;

        if (node_type_ == nullptr and types.size() == 1)
            node_type_ = types[0];

        if (node_type_ == nullptr or node_type_->is_auto())
            throw CompileError{"Ambiguously typed node expression"};

        // check all arguments are named
        for (const Argument& arg : args_)
        {
            if (not arg.is_named())
                throw CompileError{"Node expression arguments must be named"};
        }
    }

    void NodeExpression::init_subexpressions(const vector<TypePtr>& types)
    {
        for (const Argument& arg : args_)
            arg.init();
    }

    TypePtr NodeExpression::type_impl() const
    {
        return node_type_;
    }

    VarPtr NodeExpression::evaluate_impl() const
    {
        return serializer().write_node(category_, node_type_, args_, attributes());
    }

    string NodeExpression::to_string() const
    {
        return "node " + category_ + (template_type_ ? "<" + template_type_->to_string() + ">" : "") + args_.to_string();
    }
}
