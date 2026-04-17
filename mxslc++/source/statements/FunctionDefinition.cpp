//
// Created by jaket on 03/01/2026.
//

#include "FunctionDefinition.h"

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "expressions/Expression.h"
#include "runtime/TypeInfo.h"
#include "runtime/variables/Variable.h"
#include "values/ValueFactory.h"

FunctionDefinition::FunctionDefinition(
    const Runtime& runtime,
    ModifierList mods,
    TypeInfoPtr type,
    Token name,
    vector<TypeInfoPtr> template_types,
    ParameterList params,
    StmtPtr body,
    ExprPtr return_expr
) : Statement{runtime, std::move(name)},
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
            funcs_.push_back(std::make_shared<Function>(
                runtime_, mods_, std::move(type), FunctionDefinition::name(), template_type, std::move(params), std::move(body), std::move(return_expr)
            ));
        }
    }
    else
    {
        funcs_.push_back(std::make_shared<Function>(
            runtime_, std::move(mods_), std::move(type_), FunctionDefinition::name(), nullptr, std::move(params_), std::move(body_), std::move(return_expr_)
        ));
    }
}

StmtPtr FunctionDefinition::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    if (is_templated())
        throw CompileError{"Nested templated functions is not supported"s};

    TypeInfoPtr type = type_->instantiate_template_types(template_type);
    ParameterList params = params_.instantiate_template_types(template_type);
    StmtPtr body = body_->instantiate_template_types(template_type);
    ExprPtr return_expr = ::instantiate_template_types(return_expr_, template_type);
    return std::make_unique<FunctionDefinition>(runtime_, mods_, std::move(type), token_, template_types_, std::move(params), std::move(body), std::move(return_expr));
}

void FunctionDefinition::execute_impl() const
{
    for (const FuncPtr& func : funcs_)
    {
        func->init();
        if (not func->is_inline())
            write_function_definition(func);
        runtime_.scope().add_function(func);
    }
}

// outline
void FunctionDefinition::write_function_definition(const FuncPtr& func) const
{
    create_out_variables(func);
    runtime_.enter_scope();
    add_parameters_to_scope(func);
    runtime_.serializer().write_node_def_graph(func);
    runtime_.exit_scope();
}

// outline
void FunctionDefinition::create_out_variables(const FuncPtr& func) const
{
    for (const Parameter* param : func->out_parameters())
    {
        ValuePtr val = ValueFactory::create_default_value(param->type());
        runtime_.scope().add_variable(ModifierList{TokenType::Mutable}, func->nonlocal_name(*param), std::move(val));
    }
}

// outline
void FunctionDefinition::add_parameters_to_scope(const FuncPtr& func) const
{
    for (const Parameter& param : func->parameters())
    {
        if (param.is_out())
        {
            runtime_.scope().add_reference(param.name(), func->nonlocal_name(param));
        }
        else
        {
            ValuePtr val = ValueFactory::create_interface_value(param);
            VarPtr var = std::make_shared<Variable>(param.modifiers(), param.name(), std::move(val));
            runtime_.scope().add_variable(std::move(var));
        }
    }
}
