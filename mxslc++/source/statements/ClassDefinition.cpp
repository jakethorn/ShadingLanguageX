//
// Created by jaket on 05/05/2026.
//

#include "ClassDefinition.h"

#include "CompileError.h"
#include "FunctionDefinition.h"
#include "VariableDefinition.h"
#include "mtlx/MtlXSerializer.h"
#include "runtime/Function.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "utils/instantiate_template_types_utils.h"

ClassDefinition::ClassDefinition(string name, vector<TypePtr> template_types, TypePtr parent, vector<StmtPtr> body)
    : ClassDefinition{std::move(name), std::move(template_types), std::move(parent), std::move(body), Token{}}
{

}

ClassDefinition::ClassDefinition(string name, vector<TypePtr> template_types, TypePtr parent, vector<StmtPtr> body, Token token)
    : Statement{std::move(token)},
    name_{std::move(name)},
    template_types_{std::move(template_types)},
    parent_{ std::move(parent) },
    body_{std::move(body)}
{
    validate_body();
}

StmtPtr ClassDefinition::instantiate_template_types(const TypePtr& template_type) const
{
    if (is_templated())
        throw CompileError{"Nested templated classes is not supported"s};

    TypePtr parent = parent_ ? parent_->instantiate_template_types(template_type) : nullptr;
    vector<StmtPtr> body = ::instantiate_template_types(body, template_type);
    return std::make_unique<ClassDefinition>(name_, template_types_, std::move(parent), std::move(body), token_);
}

void ClassDefinition::execute_impl() const
{
    const TypePtr type = std::make_shared<Type>(name_);
    add_fields(type);
    scope().add_type(type);
    add_methods(type);
}

void ClassDefinition::validate_body() const
{
    for (const StmtPtr& stmt : body_)
    {
        const bool is_var_def = dynamic_cast<VariableDefinition*>(stmt.get());
        const bool is_func_def = dynamic_cast<FunctionDefinition*>(stmt.get());
        if (not (is_var_def or is_func_def))
        {
            throw CompileError{"Only fields and methods are allowed in class definitions"s};
        }
    }
}

void ClassDefinition::add_fields(const TypePtr& type) const
{
    for (const StmtPtr& stmt : body_)
    {
        if (const VariableDefinition* var_def = dynamic_cast<VariableDefinition*>(stmt.get()))
        {
            Field field{var_def->modifiers(), var_def->type(), var_def->name()};
            type->add_field(std::move(field));
        }
    }
}

void ClassDefinition::add_methods(const TypePtr& type) const
{
    for (const StmtPtr& stmt : body_)
    {
        if (const FunctionDefinition* func_def = dynamic_cast<FunctionDefinition*>(stmt.get()))
        {
            for (const FuncPtr& func : func_def->functions())
            {
                type->add_method(func);
                func->set_class_type(type);

                func->init();
                scope().add_function(func);

                if (not func->is_inline())
                    serializer().write_node_def_graph(func, func_def->attributes());
            }
        }
    }
}
