//
// Created by jaket on 16/04/2026.
//

#include "FunctionDefinition2.h"

#include "CompileError.h"
#include "runtime/Function2.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/TypeInfo.h"
#include "expressions/Expression.h"

FunctionDefinition2::FunctionDefinition2(
    ModifierList mods,
    TypeInfoPtr type,
    Token name,
    vector<TypeInfoPtr> template_types,
    ParameterList params,
    StmtPtr body,
    ExprPtr return_expr
) : Statement{std::move(name)},
    mods_{std::move(mods)},
    type_{std::move(type)},
    template_types_{std::move(template_types)},
    params_{std::move(params)},
    body_{std::move(body)},
    return_expr_{std::move(return_expr)}
{
    if (is_templated())
    {
        for (const TypeInfoPtr& template_type : template_types_)
        {
            type = type_->instantiate_template_types(template_type);
            params = params_.instantiate_template_types(template_type);
            body = body_->instantiate_template_types(template_type);
            return_expr = ::instantiate_template_types(return_expr_, template_type);
            funcs_.push_back(std::make_shared<Function2>(
                mods_, std::move(type), FunctionDefinition2::name(), template_type, std::move(params), std::move(body), std::move(return_expr)
            ));
        }
    }
    else
    {
        funcs_.push_back(std::make_shared<Function2>(
            std::move(mods_), std::move(type_), FunctionDefinition2::name(), nullptr, std::move(params_), std::move(body_), std::move(return_expr_)
        ));
    }
}

StmtPtr FunctionDefinition2::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    if (is_templated())
        throw CompileError{"Nested templated functions is not supported"s};

    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ParameterList params = params_.instantiate_template_types(template_type);
    StmtPtr body = body_->instantiate_template_types(template_type);
    ExprPtr return_expr = ::instantiate_template_types(return_expr_, template_type);
    return std::make_unique<FunctionDefinition2>(mods_, std::move(type), token_, template_types_, std::move(params), std::move(body), std::move(return_expr));
}

void FunctionDefinition2::execute_impl() const
{
    for (const FuncPtr2& func : funcs_)
    {
        func->init();
        if (not func->is_inline())
            write_function_definition(func);
        runtime().scope().add_function(func);
    }
}

void FunctionDefinition2::write_function_definition(const FuncPtr2& func) const
{
    runtime().enter_scope();
    runtime().serializer().write_node_def_graph(func);
    runtime().exit_scope();
}
