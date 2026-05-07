//
// Created by jaket on 05/05/2026.
//

#include "ClassDefinition.h"

#include "CompileError.h"
#include "FunctionDefinition.h"
#include "VariableDefinition.h"
#include "mtlx/MtlXSerializer.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/ValueFactory.h"

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

}

StmtPtr ClassDefinition::instantiate_template_types(const TypePtr& template_type) const
{
    if (is_templated())
        throw CompileError{"Nested templated classes is not supported"s};

    vector<Field> fields = ::instantiate_template_types(fields_, template_type);
    vector<StmtPtr> funcs = ::instantiate_template_types(func_defs_, template_type);
    return std::make_unique<ClassDefinition>(name_, parent_, template_types_, std::move(fields), std::move(funcs), token_);
}

void ClassDefinition::execute_impl() const
{
    const TypePtr type = std::make_shared<Type>(token_.lexeme(), fields_);
    scope().add_type(type);

    Runtime::get().enter_scope();

    const VarPtr this_ = ValueFactory::create_default_value(type);
    scope().add_variable("__this__"s, std::move(this_));

    for (const StmtPtr& stmt : body_)
    {
        if (const FunctionDefinition* func_def = dynamic_cast<FunctionDefinition*>(stmt.get()))
        {
            for (const FuncPtr& func : func_def->functions())
            {
                func->init();
                if (not func->is_inline())
                    serializer().write_node_def_graph(func, func_def->attributes());
                type->add_method(func);
            }
        }
        else if (const VariableDefinition* var_def = dynamic_cast<VariableDefinition*>(stmt.get()))
        {
            // move field init here
        }
        else
        {
            throw CompileError{"Only fields and methods are allowed in class definitions"s};
        }
    }

    Runtime::get().exit_scope();
}
