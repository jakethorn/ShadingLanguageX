//
// Created by jaket on 03/01/2026.
//

#include "FunctionDefinition.h"

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "values/InterfaceValue.h"
#include "expressions/Expression.h"

FunctionDefinition::FunctionDefinition(
    const Runtime& runtime,
    vector<string> modifiers,
    Type type,
    Token name,
    vector<Type> template_types,
    ParameterList params,
    vector<StmtPtr> body,
    ExprPtr return_expr
) : Statement{runtime},
    mods_{std::move(modifiers)},
    type_{std::move(type)},
    name_{std::move(name)},
    template_types_{std::move(template_types)},
    params_{std::move(params)},
    body_{std::move(body)},
    return_expr_{std::move(return_expr)}
{
    if (is_templated())
    {
        for (const Type& template_type : template_types_)
        {
            type = type_.instantiate_template_types(template_type);
            params = params_.instantiate_template_types(template_type);
            body = Type::instantiate_template_types(body_, template_type);
            return_expr = return_expr_ ? return_expr_->instantiate_template_types(template_type) : nullptr;
            funcs_.push_back(std::make_shared<Function>(
                mods_, std::move(type), name_, std::move(template_type), std::move(params), std::move(body), std::move(return_expr)
            ));
        }
    }
    else
    {
        funcs_.push_back(std::make_shared<Function>(
            std::move(mods_), std::move(type_), std::move(name_), std::nullopt, std::move(params_), std::move(body_), std::move(return_expr_)
        ));
    }
}

StmtPtr FunctionDefinition::instantiate_template_types(const Type& template_type) const
{
    if (is_templated())
        throw CompileError{name_, "Nested templated functions is not supported"s};

    Type type = type_.instantiate_template_types(template_type);
    ParameterList params = params_.instantiate_template_types(template_type);
    vector body = Type::instantiate_template_types(body_, template_type);
    ExprPtr return_expr = return_expr_ ? return_expr_->instantiate_template_types(template_type) : nullptr;
    return std::make_unique<FunctionDefinition>(runtime_, mods_, std::move(type), name_, template_types_, std::move(params), std::move(body), std::move(return_expr));
}

void FunctionDefinition::execute() const
{
    for (const FuncPtr& func : funcs_)
    {
        if (not func->is_inline())
            write_function_definition(*func);
        runtime_.scope().add_function(func);
    }
}

void FunctionDefinition::write_function_definition(const Function& func) const
{
    runtime_.enter_scope();
    for (const Parameter& param : func.parameters())
    {
        ValuePtr val = std::make_unique<InterfaceValue>(param.name(), param.type());
        Variable var{{}, param.type(), param.name(), std::move(val)};
        runtime_.scope().add_variable(std::move(var));
    }
    runtime_.serializer().write_node_def_graph(func);
    runtime_.exit_scope();
}
