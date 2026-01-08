//
// Created by jaket on 03/01/2026.
//

#include "FunctionDefinition.h"

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/values/InterfaceValue.h"
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
    modifiers_{std::move(modifiers)},
    type_{std::move(type)},
    name_{std::move(name)},
    template_types_{std::move(template_types)},
    params_{std::move(params)},
    body_{std::move(body)},
    return_expr_{std::move(return_expr)}
{

}

StmtPtr FunctionDefinition::instantiate_templated_types(const Type& template_type) const
{
    if (is_templated())
        throw CompileError{name_, "Nested templated functions is not supported"s};

    Type type = type_.instantiate_template_type(template_type);
    ParameterList params = params_.instantiate_templated_types(template_type);

    vector<StmtPtr> body;
    body.reserve(body_.size());
    for (const StmtPtr& stmt : body_)
        body.push_back(stmt->instantiate_templated_types(template_type));

    ExprPtr return_expr = return_expr_ ? return_expr_->instantiate_templated_types(template_type) : nullptr;

    return std::make_unique<FunctionDefinition>(runtime_, modifiers_, std::move(type), name_, template_types_, std::move(params), std::move(body), std::move(return_expr));
}

vector<Function> FunctionDefinition::functions()
{
    vector<Function> funcs;
    if (is_templated())
    {
        for (const Type& template_type : template_types_)
        {
            Type type = type_.instantiate_template_type(template_type);
            ParameterList params = params_.instantiate_templated_types(template_type);
            vector<StmtPtr> body;
            body.reserve(body_.size());
            for (const StmtPtr& stmt : body_)
                body.push_back(stmt->instantiate_templated_types(template_type));
            ExprPtr return_expr = return_expr_ ? return_expr_->instantiate_templated_types(template_type) : nullptr;
            funcs.emplace_back(modifiers_, std::move(type), name_, template_type, std::move(params), std::move(body), std::move(return_expr));
        }
    }
    else
    {
        funcs.emplace_back(std::move(modifiers_), std::move(type_), std::move(name_), std::nullopt, std::move(params_), std::move(body_), std::move(return_expr_));
    }
    return funcs;
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
    runtime_.serializer().write_function(func);
    runtime_.exit_scope();
}

void FunctionDefinition::execute()
{
    for (Function& func : functions())
    {
        if (not func.is_inline())
            write_function_definition(func);
        runtime_.scope().add_function(std::move(func));
    }
}
