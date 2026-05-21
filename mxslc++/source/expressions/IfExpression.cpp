//
// Created by jaket on 21/05/2026.
//

#include "IfExpression.h"
#include "runtime/Type.h"
#include "utils/parse_utils.h"
#include "utils/str_utils.h"

IfExpression::IfExpression(ExprPtr cond_expr, ExprPtr then_expr, ExprPtr else_expr, Token token)
    : FunctionCall{std::move(token)}, cond_expr_{std::move(cond_expr)}, then_expr_{std::move(then_expr)}, else_expr_{std::move(else_expr)}
{

}

ExprPtr IfExpression::instantiate_template_types(const TypePtr& template_type) const
{
    ExprPtr cond_expr = cond_expr_->instantiate_template_types(template_type);
    ExprPtr then_expr = then_expr_->instantiate_template_types(template_type);
    ExprPtr else_expr = else_expr_->instantiate_template_types(template_type);
    return std::make_unique<IfExpression>(std::move(cond_expr), std::move(then_expr), std::move(else_expr), token_);
}

void IfExpression::init_subexpressions(const vector<TypePtr>& types)
{
    cond_expr_->init(Type::Bool);
    then_expr_->init(types);
    else_expr_->init(types);
}

namespace
{
    vector<string> arg_lines(const TypePtr& type, string index_chain, const size_t index)
    {
        index_chain = index_chain + "[" + str(index) + "]";

        if (type->is_primitive())
            return vector<string>{std::move(index_chain)};

        vector<string> result;
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            vector<string> lines = arg_lines(type->field_type(i), std::move(index_chain), i);
            result.insert(result.cend(), lines.begin(), lines.end());
        }
        return result;
    }

    vector<string> arg_lines(const TypePtr& type)
    {
        if (type->is_primitive())
            return vector<string>{""s};

        vector<string> result;
        for (size_t i = 0; i < type->field_count(); ++i)
        {
            vector<string> lines = arg_lines(type->field_type(i), ""s, i);
            result.insert(result.cend(), lines.begin(), lines.end());
        }
        return result;
    }
}

void IfExpression::init_impl(const vector<TypePtr>& types)
{
    const string type_name = then_expr_->type()->str();
    const string code = R"(
        inline )" + type_name + R"( __if__(boolean condition, )" + type_name + R"( then_value, )" + type_name + R"( else_value)
        {
            return {
                ifequal(condition, true, then_value[0], else_value[0]),
                ifequal(condition, true, then_value[1], else_value[1]),
            };
        }
    )";

    func_ = parse_function(code);
}
