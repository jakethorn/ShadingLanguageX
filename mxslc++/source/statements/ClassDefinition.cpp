//
// Created by jaket on 05/05/2026.
//

#include "ClassDefinition.h"

#include "CompileError.h"
#include "FunctionDefinition.h"
#include "mtlx/MtlXSerializer.h"
#include "runtime/Runtime.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "utils/instantiate_template_types_utils.h"
#include "values/ValueFactory.h"

ClassDefinition::ClassDefinition(string name, string parent, vector<TypePtr> template_types, vector<Field> fields, vector<StmtPtr> func_defs)
    : ClassDefinition{std::move(name), std::move(parent), std::move(template_types), std::move(fields), std::move(func_defs), Token{}}
{

}

ClassDefinition::ClassDefinition(string name, string parent, vector<TypePtr> template_types, vector<Field> fields, vector<StmtPtr> func_defs, Token token)
    : Statement{std::move(token)},
    name_{std::move(name)},
    parent_{std::move(parent)},
    template_types_{std::move(template_types)},
    fields_{std::move(fields)},
    func_defs_{std::move(func_defs)}
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

    for (const StmtPtr& stmt : func_defs_)
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
        else
        {
            throw CompileError{"Only fields and methods are allowed in class definitions"s};
        }
    }

    Runtime::get().exit_scope();
}
