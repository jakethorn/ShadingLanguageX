//
// Created by jaket on 01/12/2025.
//

#include "Function.h"

#include "CompileError.h"
#include "statements/Statement.h"
#include "utils/template_utils.h"

Function::Function(vector<string> modifiers, Type type, Token name, optional<Type> template_type, ParameterList params, vector<StmtPtr> body)
    : modifiers_{std::move(modifiers)},
    type_{std::move(type)},
    name_{std::move(name)},
    template_type_{std::move(template_type)},
    params_{std::move(params)},
    body_{std::move(body)}
{
    static const vector valid_modifiers{"inline"s};
    for (const string& modifier : modifiers_)
        if (not contains(valid_modifiers, modifier))
            throw CompileError{name_, "'" + modifier + "' is not a valid function modifier"};
}

Function::Function(Function&& other) noexcept
    : modifiers_(std::move(other.modifiers_)),
    type_(std::move(other.type_)),
    name_(std::move(other.name_)),
    template_type_(std::move(other.template_type_)),
    params_(std::move(other.params_)),
    body_(std::move(other.body_))
{

}

Function& Function::operator=(Function&& other) noexcept
{
    if (this != &other)
    {
        modifiers_     = std::move(other.modifiers_);
        type_          = std::move(other.type_);
        name_          = std::move(other.name_);
        template_type_ = std::move(other.template_type_);
        params_        = std::move(other.params_);
        body_          = std::move(other.body_);
    }
    return *this;
}

Function::~Function() = default;
