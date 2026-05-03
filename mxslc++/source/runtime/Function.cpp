//
// Created by jaket on 16/04/2026.
//

#include "Function.h"

#include "Runtime.h"
#include "Scope.h"
#include "Type.h"
#include "expressions/Expression.h"
#include "statements/Statement.h"

Function::Function(
    ModifierList mods,
    TypePtr return_type,
    string name,
    TypePtr template_type,
    ParameterList params,
    mx::NodeDefPtr node_def
) : mods_{std::move(mods)},
    return_type_{std::move(return_type)},
    name_{std::move(name)},
    template_type_{std::move(template_type)},
    params_{std::move(params)}
{
    set_node_def(std::move(node_def));
}

Function::Function(
    ModifierList mods,
    TypePtr return_type,
    string name,
    TypePtr template_type,
    ParameterList params,
    StmtPtr body,
    ExprPtr return_expr
) : mods_{std::move(mods)},
    return_type_{std::move(return_type)},
    name_{std::move(name)},
    template_type_{std::move(template_type)},
    params_{std::move(params)},
    body_{std::move(body)},
    return_expr_{std::move(return_expr)}
{
    mods_.validate(TokenType::Inline, TokenType::Default);

    if (return_type_ == Type::Void and return_expr_ != nullptr)
        throw CompileError{"Void function '" + name_ + "' has a return statement"s};
    if (return_type_ != Type::Void and return_expr_ == nullptr)
        throw CompileError{"Non-void function '" + name_ + "' does not have a return statement"s};
}

Function::Function(Function&& other) noexcept
    : mods_{std::move(other.mods_)},
    return_type_{std::move(other.return_type_)},
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

Function::~Function() = default;

bool Function::is_void() const
{
    return return_type_ == Type::Void;
}

size_t Function::min_arity() const
{
    size_t arity = 0;
    for (const Parameter& param : params_)
    {
        if (not param.has_default_value())
            ++arity;
    }

    return arity;
}

void Function::set_node_def(mx::NodeDefPtr node_def)
{
    node_def_ = std::move(node_def);
}

vector<string> Function::output_names() const
{
    if (is_defined())
        return {};

    vector<string> names;
    names.reserve(node_def_->getOutputCount());
    for (const mx::OutputPtr& o : node_def_->getActiveOutputs())
        names.push_back(o->getName());
    return names;
}

void Function::init()
{
    if (return_type_ == Type::Void)
        return_type_ = std::make_shared<ResolvedTypeInfo>(Type::Void);
    else
        return_type_ = Runtime::get().scope().resolve_type(return_type_);

    if (template_type_)
        template_type_ = Runtime::get().scope().resolve_type(template_type_);

    params_.init();

    is_initialized_ = true;
}

VarPtr Function::invoke() const
{
    body_->execute();

    if (is_void())
    {
        return nullptr;
    }
    else
    {
        return_expr_->init(return_type_);
        return return_expr_->evaluate();
    }
}

string Function::nonlocal_name(const Parameter& param) const
{
    return name_ + "_" + (template_type_ ? template_type_->name() : ""s) + "__" + param.name();
}

string Function::str() const
{
    string result;
    result += mods_.str();
    result += return_type_->str();
    result += " " + name_;
    if (template_type_)
        result += "<" + template_type_->str() + ">";
    result += "(";
    result += params_.str();
    result += ")";
    return result;
}
