//
// Created by jaket on 03/01/2026.
//

#include "FunctionDefinition.h"

#include "CompileError.h"
#include "runtime/Runtime.h"

StmtPtr FunctionDefinition::instantiate_templated_types(const Type& template_type) const
{
    if (is_templated())
        throw CompileError{name_, "Nested templated functions is not supported"};

    Type type = type_.instantiate_template_type(template_type);
    ParameterList params = params_.instantiate_templated_types(template_type);

    vector<StmtPtr> body;
    body.reserve(body_.size());
    for (const StmtPtr& stmt : body_)
        body.push_back(stmt->instantiate_templated_types(template_type));

    return std::make_unique<FunctionDefinition>(runtime_, modifiers_, std::move(type), name_, template_types_, std::move(params), std::move(body));
}

void FunctionDefinition::init()
{
    if (is_templated())
    {
        for (const Type& template_type : template_types_)
        {
            Type type = type_.instantiate_template_type(template_type);
            ParameterList params = params_.instantiate_templated_types(template_type);

            vector<StmtPtr> body;
            body.reserve(body_.size());
            for (const StmtPtr& stmt : body_)
            {
                StmtPtr stmt_inst = stmt->instantiate_templated_types(template_type);
                stmt_inst->init();
                body.push_back(std::move(stmt_inst));
            }

            funcs_.emplace_back(modifiers_, std::move(type), name_, template_type, std::move(params), std::move(body));
        }
    }
    else
    {
        for (const StmtPtr& stmt : body_)
            stmt->init();
        funcs_.emplace_back(std::move(modifiers_), std::move(type_), std::move(name_), std::nullopt, std::move(params_), std::move(body_));
    }
}

void FunctionDefinition::execute()
{
    for (Function& func : funcs_)
    {
        runtime_.serializer().write_function(func);
        runtime_.scope().add_function(std::move(func));
    }
}
