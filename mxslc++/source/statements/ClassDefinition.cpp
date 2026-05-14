//
// Created by jaket on 05/05/2026.
//

#include "ClassDefinition.h"

#include "CompileError.h"
#include "ConstructorDefinition.h"
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
    add_methods_and_constructors(type);
}

void ClassDefinition::validate_body() const
{
    for (const StmtPtr& stmt : body_)
    {
        const bool is_var_def = dynamic_cast<VariableDefinition*>(stmt.get());
        const bool is_func_def = dynamic_cast<FunctionDefinition*>(stmt.get());
        const bool is_ctor_def = dynamic_cast<ConstructorDefinition*>(stmt.get());
        if (not (is_var_def or is_func_def or is_ctor_def))
        {
            throw CompileError{"Only fields, methods and constructors are allowed in class definitions"s};
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

void ClassDefinition::add_methods_and_constructors(const TypePtr& type) const
{
    for (const StmtPtr& stmt : body_)
    {
        if (FunctionDefinition* func_def = dynamic_cast<FunctionDefinition*>(stmt.get()))
        {
            add_method(type, func_def);
        }
        else if (ConstructorDefinition* ctor_def = dynamic_cast<ConstructorDefinition*>(stmt.get()))
        {
            add_constructor(type, ctor_def);
        }
    }
}

void ClassDefinition::add_method(const TypePtr& type, FunctionDefinition* func_def) const
{
    for (const FuncPtr& func : func_def->functions())
    {
        type->add_method(func);
        func->set_class_type(type);
    }

    func_def->execute();
}

void ClassDefinition::add_constructor(const TypePtr& type, ConstructorDefinition* ctor_def) const
{
    if (type->name() != ctor_def->class_name())
        throw CompileError{"Constructor name does not match class name"s};
    ctor_def->execute();
}
