//
// Created by jaket on 16/04/2026.
//

#include "Function2.h"

#include "Runtime.h"
#include "Scope.h"
#include "TypeInfo.h"
#include "expressions/Expression.h"
#include "statements/Statement.h"

Function2::Function2(
    ModifierList mods,
    TypeInfoPtr return_type,
    string name,
    TypeInfoPtr template_type,
    ParameterList params,
    mx::NodeDefPtr node_def
) : mods_{std::move(mods)},
    type_{std::move(return_type)},
    name_{std::move(name)},
    template_type_{std::move(template_type)},
    params_{std::move(params)}
{
    set_node_def(std::move(node_def));
}

Function2::Function2(
    ModifierList mods,
    TypeInfoPtr return_type,
    string name,
    TypeInfoPtr template_type,
    ParameterList params,
    StmtPtr body,
    ExprPtr return_expr
) : mods_{std::move(mods)},
    type_{std::move(return_type)},
    name_{std::move(name)},
    template_type_{std::move(template_type)},
    params_{std::move(params)},
    body_{std::move(body)},
    return_expr_{std::move(return_expr)}
{
    mods_.validate(TokenType::Inline, TokenType::Default);

    if (type_ == TypeInfo::Void and return_expr_ != nullptr)
        throw CompileError{"Void function '" + name_ + "' has a return statement"s};
    if (type_ != TypeInfo::Void and return_expr_ == nullptr)
        throw CompileError{"Non-void function '" + name_ + "' does not have a return statement"s};
}

Function2::Function2(Function2&& other) noexcept
    : mods_{std::move(other.mods_)},
    type_{std::move(other.type_)},
    name_{std::move(other.name_)},
    template_type_{std::move(other.template_type_)},
    params_{std::move(other.params_)},
    body_{std::move(other.body_)},
    return_expr_{std::move(other.return_expr_)},
    node_def_{std::move(other.node_def_)},
    is_initialized_{other.is_initialized_},
    nonlocal_inputs_{std::move(other.nonlocal_inputs_)},
    nonlocal_outputs_{std::move(other.nonlocal_outputs_)}
{

}

Function2::~Function2() = default;

bool Function2::is_void() const
{
    return type_ == TypeInfo::Void;
}

size_t Function2::min_arity() const
{
    size_t arity = 0;
    for (const Parameter& param : params_)
    {
        if (not param.has_default_value())
            ++arity;
    }

    return arity;
}

void Function2::set_node_def(mx::NodeDefPtr node_def)
{
    node_def_ = std::move(node_def);
}

void Function2::init()
{
    if (type_ == TypeInfo::Void)
        type_ = std::make_shared<ResolvedTypeInfo>(TypeInfo::Void);
    else
        type_ = Runtime::get().scope().resolve_type(type_);

    if (template_type_)
        template_type_ = Runtime::get().scope().resolve_type(template_type_);

    params_.init();

    is_initialized_ = true;
}

VarPtr2 Function2::invoke() const
{
    body_->execute();

    if (is_void())
    {
        return nullptr;
    }
    else
    {
        return_expr_->init(type_);
        return return_expr_->evaluate();
    }
}

string Function2::nonlocal_name(const Parameter& param) const
{
    return name_ + "_" + (template_type_ ? template_type_->name() : ""s) + "__" + param.name();
}

string Function2::str() const
{
    string result;
    result += mods_.str();
    result += type_->str();
    result += " " + name_;
    if (template_type_)
        result += "<" + template_type_->str() + ">";
    result += "(";
    result += params_.str();
    result += ")";
    return result;
}
