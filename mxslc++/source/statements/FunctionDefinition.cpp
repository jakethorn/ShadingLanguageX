//
// Created by jaket on 03/01/2026.
//

#include "FunctionDefinition.h"

#include "CompileError.h"
#include "runtime/Runtime.h"
#include "runtime/values/InterfaceValue.h"

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

    return std::make_unique<FunctionDefinition>(runtime_, modifiers_, std::move(type), name_, template_types_, std::move(params), std::move(body));
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

            funcs.emplace_back(modifiers_, std::move(type), name_, template_type, std::move(params), std::move(body));
        }
    }
    else
    {
        funcs.emplace_back(std::move(modifiers_), std::move(type_), std::move(name_), std::nullopt, std::move(params_), std::move(body_));
    }
    return funcs;
}

void FunctionDefinition::execute()
{
    for (Function& func : functions())
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

        runtime_.scope().add_function(std::move(func));
    }
}
