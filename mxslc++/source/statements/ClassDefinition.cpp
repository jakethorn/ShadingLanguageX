//
// Created by jaket on 05/05/2026.
//

#include "statements/ClassDefinition.h"

#include "statements/ConstructorDefinition.h"
#include "statements/FunctionDefinition.h"
#include "statements/VariableDefinition.h"
#include "serialize/Serializer.h"
#include "runtime/Function.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "runtime/utils/monomorphize.h"
#include "errors/CompileError.h"
#include "runtime/interface.h"
#include "statements/interface.h"
#include "statements/MultiVariableDefinition.h"

namespace mxslc::statements
{
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

    StmtPtr ClassDefinition::monomorphize(const TypePtr& template_type) const
    {
        if (is_templated())
            throw CompileError{"Nested templated classes is not supported"};

        auto&& [parent, body] = runtime_utils::monomorphize_all(template_type, parent_, body_);
        return create_statement<ClassDefinition>(name_, template_types_, std::move(parent), std::move(body), token_);
    }

    void ClassDefinition::execute_impl() const
    {
        const TypePtr class_type = create_type(name_);
        add_fields(class_type);
        scope().add_type(class_type);
        add_methods_and_constructors(class_type);
    }

    void ClassDefinition::validate_body() const
    {
        for (const StmtPtr& stmt : body_)
        {
            const bool is_var_def = dynamic_cast<VariableDefinition*>(stmt.get());
            const bool is_multi_var_def = dynamic_cast<MultiVariableDefinition*>(stmt.get());
            const bool is_func_def = dynamic_cast<FunctionDefinition*>(stmt.get());
            const bool is_ctor_def = dynamic_cast<ConstructorDefinition*>(stmt.get());
            if (not (is_var_def or is_multi_var_def or is_func_def or is_ctor_def))
            {
                throw CompileError{"Only fields, methods and constructors are allowed in class definitions"};
            }
        }
    }

    void ClassDefinition::add_fields(const TypePtr& class_type) const
    {
        for (const StmtPtr& stmt : body_)
        {
            if (const VariableDefinition* var_def = dynamic_cast<VariableDefinition*>(stmt.get()))
            {
                Field field{var_def->modifiers(), var_def->type(), var_def->name()};
                class_type->add_field(std::move(field));
            }
            else if (const MultiVariableDefinition* multi_var_def = dynamic_cast<MultiVariableDefinition*>(stmt.get()))
            {
                for (const Field& field : multi_var_def->type()->fields())
                    class_type->add_field(field);
            }
        }
    }

    void ClassDefinition::add_methods_and_constructors(const TypePtr& class_type) const
    {
        for (const StmtPtr& stmt : body_)
        {
            if (FunctionDefinition* func_def = dynamic_cast<FunctionDefinition*>(stmt.get()))
            {
                add_method(class_type, func_def);
            }
            else if (ConstructorDefinition* ctor_def = dynamic_cast<ConstructorDefinition*>(stmt.get()))
            {
                add_constructor(class_type, ctor_def);
            }
        }
    }

    void ClassDefinition::add_method(const TypePtr& class_type, FunctionDefinition* func_def) const
    {
        for (const FuncPtr& func : func_def->functions())
        {
            class_type->add_method(func);
            func->set_class_type(class_type);
        }

        func_def->execute();
    }

    void ClassDefinition::add_constructor(const TypePtr& class_type, ConstructorDefinition* ctor_def) const
    {
        if (class_type->name() != ctor_def->class_name())
            throw CompileError{"Constructor name does not match class name"};
        ctor_def->execute();
    }

    string ClassDefinition::to_string() const
    {
        string result;
        result += "class " + name_;

        if (not template_types_.empty())
            result += "<" + join(template_types_, ", ") + ">";

        if (parent_)
            result += " : " + parent_->to_string();

        result += "\n{\n\t";

        result += join(body_, "\n\n\t");

        result += "\n}";

        return result;
    }
}
