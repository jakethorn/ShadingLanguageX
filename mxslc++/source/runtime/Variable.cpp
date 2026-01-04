//
// Created by jaket on 03/12/2025.
//

#include "Variable.h"

#include "CompileError.h"
#include "values/Value.h"
#include "utils/template_utils.h"

Variable::Variable(vector<string> modifiers, Type type, Token name)
    : Variable{std::move(modifiers), std::move(type), std::move(name), nullptr}
{

}

Variable::Variable(vector<string> modifiers, Type type, Token name, ValuePtr value)
    : modifiers_{std::move(modifiers)},
    type_{std::move(type)},
    name_{std::move(name)},
    value_{std::move(value)}
{
    static const vector valid_modifiers{"const"s, "mutable"s, "global"s};
    for (const string& modifier : modifiers_)
        if (not contains(valid_modifiers, modifier))
            throw CompileError{name_, "'" + modifier + "' is not a valid variable modifier"};

    if (is_const() and is_mutable())
        throw CompileError{name_, "Variables cannot be both const and mutable"s};
}
