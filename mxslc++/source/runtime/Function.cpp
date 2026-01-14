//
// Created by jaket on 01/12/2025.
//

#include "Function.h"

#include "CompileError.h"
#include "statements/Statement.h"
#include "utils/template_utils.h"
#include "expressions/Expression.h"
#include "mtlx/mtlx_utils.h"
#include "utils/str_utils.h"

Function::Function(
    Type type,
    Token name,
    optional<Type> template_type,
    ParameterList params,
    vector<string> output_names
) : type_{std::move(type)},
    name_{std::move(name)},
    template_type_{std::move(template_type)},
    params_{std::move(params)},
    output_names_({std::move(output_names)})
{

}

Function::Function(
    vector<string> mods,
    Type type,
    Token name,
    optional<Type> template_type,
    ParameterList params,
    vector<StmtPtr> body,
    ExprPtr return_expr
) : mods_{std::move(mods)},
    type_{std::move(type)},
    name_{std::move(name)},
    template_type_{std::move(template_type)},
    params_{std::move(params)},
    body_{std::move(body)},
    return_expr_{std::move(return_expr)}
{
    static const vector valid_mods{"inline"s};
    for (const string& mod : mods_)
        if (not contains(valid_mods, mod))
            throw CompileError{name_, "'" + mod + "' is not a valid function modifier"};

    if (type_ == "void"s and return_expr_ != nullptr)
        throw CompileError{name_, "Cannot return a value from a void function"s};

    for (size_t i = 0; i < type_.subtype_count(); ++i)
        output_names_.push_back(port_name("out"s, i));
}

Function::Function(Function&& other) noexcept
    : mods_{std::move(other.mods_)},
    type_{std::move(other.type_)},
    name_{std::move(other.name_)},
    template_type_{std::move(other.template_type_)},
    params_{std::move(other.params_)},
    body_{std::move(other.body_)},
    return_expr_{std::move(other.return_expr_)},
    output_names_{std::move(other.output_names_)}
{

}

Function& Function::operator=(Function&& other) noexcept
{
    if (this != &other)
    {
        mods_     = std::move(other.mods_);
        type_          = std::move(other.type_);
        name_          = std::move(other.name_);
        template_type_ = std::move(other.template_type_);
        params_        = std::move(other.params_);
        body_          = std::move(other.body_);
        return_expr_   = std::move(other.return_expr_);
        output_names_  = std::move(other.output_names_);
    }
    return *this;
}

Function::~Function() = default;
