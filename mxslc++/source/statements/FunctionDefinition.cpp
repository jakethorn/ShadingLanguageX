//
// Created by jaket on 16/04/2026.
//

#include "FunctionDefinition.h"

#include "CompileError.h"
#include "runtime/Function.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "expressions/Expression.h"

FunctionDefinition::FunctionDefinition(
    ModifierList mods,
    TypePtr type,
    string name,
    vector<TypePtr> template_types,
    ParameterList params,
    StmtPtr body,
    ExprPtr return_expr
) : FunctionDefinition{
    std::move(mods),
    std::move(type),
    std::move(name),
    std::move(template_types),
    std::move(params),
    std::move(body),
    std::move(return_expr),
    Token{}
}
{

}

FunctionDefinition::FunctionDefinition(
    ModifierList mods,
    TypePtr type,
    string name,
    vector<TypePtr> template_types,
    ParameterList params,
    StmtPtr body,
    ExprPtr return_expr,
    Token token
) : Statement{std::move(token)},
    mods_{std::move(mods)},
    type_{std::move(type)},
    name_{std::move(name)},
    template_types_{std::move(template_types)},
    params_{std::move(params)},
    body_{std::move(body)},
    return_expr_{std::move(return_expr)}
{
    if (is_templated())
    {
        for (const TypePtr& template_type : template_types_)
        {
            type = type_->instantiate_template_types(template_type);
            params = params_.instantiate_template_types(template_type);
            body = body_->instantiate_template_types(template_type);
            return_expr = ::instantiate_template_types(return_expr_, template_type);
            funcs_.push_back(std::make_shared<Function>(
                mods_, std::move(type), name_, template_type, std::move(params), std::move(body), std::move(return_expr)
            ));
        }
    }
    else
    {
        funcs_.push_back(std::make_shared<Function>(
            std::move(mods_), std::move(type_), std::move(name_), nullptr, std::move(params_), std::move(body_), std::move(return_expr_)
        ));
    }
}

void FunctionDefinition::set_attributes(AttributeList attrs)
{
    attrs_ = std::move(attrs);
}

StmtPtr FunctionDefinition::instantiate_template_types(const TypePtr& template_type) const
{
    if (is_templated())
        throw CompileError{"Nested templated functions is not supported"s};

    TypePtr type = type_->instantiate_template_types(template_type);
    ParameterList params = params_.instantiate_template_types(template_type);
    StmtPtr body = body_->instantiate_template_types(template_type);
    ExprPtr return_expr = ::instantiate_template_types(return_expr_, template_type);
    return std::make_unique<FunctionDefinition>(mods_, std::move(type), name_, template_types_, std::move(params), std::move(body), std::move(return_expr), token_);
}

void FunctionDefinition::execute_impl() const
{
    for (const FuncPtr& func : funcs_)
    {
        func->init();
        scope().add_function(func);

        if (not func->is_inline())
            serializer().write_node_def_graph(func, attrs_);
    }
}
