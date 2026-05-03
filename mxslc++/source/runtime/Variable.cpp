//
// Created by jaket on 16/04/2026.
//

#include "Variable.h"

#include "Runtime.h"
#include "Scope.h"
#include "expressions/ExpressionFactory.h"
#include "mtlx/mtlx_utils.h"
#include "values/NodeValue.h"

Variable::Variable(ModifierList mods, TypePtr type) : type_{std::move(type)}
{
    set_modifiers(std::move(mods));
}

bool Variable::is_const() const
{
    return mods_.contains(TokenType::Const);
}

bool Variable::is_mutable() const
{
    return mods_.contains(TokenType::Mutable);
}

const ModifierList& Variable::modifiers() const
{
    return mods_;
}

void Variable::set_modifiers(ModifierList mods)
{
    mods_ = std::move(mods);
    mods_.validate(TokenType::Const, TokenType::Mutable, TokenType::Global);
    if (is_const() and is_mutable())
        throw CompileError{"Variables cannot be both const and mutable"s};
}

const TypePtr& Variable::type() const
{
    return type_;
}

const string& Variable::name() const
{
    return name_;
}

void Variable::set_name(string name)
{
    name_ = std::move(name);
    for (size_t i = 0; i < children_.size(); ++i)
    {
        children_[i]->set_name(get_port_name(name_, i));
    }
}

bool Variable::is_assignable() const
{
    if (not is_initialized_)
        return true;
    if (is_const())
        return false;
    if (has_parent() and parent()->is_const())
        return false;
    if (is_mutable())
        return true;
    if (has_parent() and parent()->is_mutable())
        return true;
    return false;
}

bool Variable::is_temporary() const
{
    if (has_parent())
        return parent()->is_temporary();
    return name_.empty();
}

bool Variable::has_parent() const
{
    return parent_.lock() != nullptr;
}

VarPtr Variable::parent() const
{
    return parent_.lock();
}

size_t Variable::child_count() const
{
    return children_.size();
}

VarPtr Variable::child(const size_t index)
{
    return children_.at(index);
}

VarPtr Variable::child(const string& field_name)
{
    return child(type_->field_index(field_name));
}

bool Variable::has_value() const
{
    return value_ != nullptr;
}

ValuePtr Variable::value()
{
    if (is_temporary() or is_local())
    {
        return value_;
    }
    else
    {
        return Runtime::get().serializer().write_node_def_input(shared_from_this());
    }
}

void Variable::copy_value(const VarPtr& other)
{
    if (is_initialized_)
    {
        if (is_temporary())
            throw CompileError{"Cannot assign value to temporary variable"s};
        if (not is_assignable())
            throw CompileError{"Cannot assign value to non-mutable variable '" + name_ + "'"};
    }

    if (other->has_value())
    {
        set_value(other->value());
    }
    else
    {
        copy_children(other->children_);
    }
}

void Variable::uninitialize()
{
    is_initialized_ = false;
    for (const VarPtr& child : children_)
    {
        child->uninitialize();
    }
}

void Variable::add_to_scope(string name)
{
    Runtime::get().scope().add_variable(std::move(name), shared_from_this());
}

bool Variable::is_local()
{
    return Runtime::get().scope().is_variable_local(shared_from_this());
}

string Variable::str() const
{
    if (has_value())
    {
        return value_->str();
    }
    else
    {
        string result = "{";
        for (const VarPtr& child : children_)
        {
            result += "\n\t" + child->str() + "\n";
        }
        return result + "}";
    }
}

VarPtr Variable::create(ModifierList mods, TypePtr type, const vector<VarPtr>& children)
{
    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->copy_children(children);
    return var;
}

VarPtr Variable::create(ModifierList mods, TypePtr type, ValuePtr value)
{
    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->set_value(std::move(value));
    return var;
}

VarPtr Variable::create(ModifierList mods, TypePtr type, const VarPtr& value)
{
    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->copy_value(value);
    return var;
}

VarPtr Variable::create(TypePtr type, const vector<VarPtr>& children)
{
    return create(ModifierList{}, std::move(type), children);
}

VarPtr Variable::create(TypePtr type, ValuePtr value)
{
    return create(ModifierList{}, std::move(type), std::move(value));
}

VarPtr Variable::create(TypePtr type, const VarPtr& value)
{
    return create(ModifierList{}, std::move(type), value);
}

VarPtr Variable::create(ValuePtr value)
{
    TypePtr type = value->type();
    return create(ModifierList{}, std::move(type), std::move(value));
}

VarPtr Variable::create(const VarPtr& value)
{
    return create(ModifierList{}, value->type(), value);
}

void Variable::set_parent(weak_ptr<Variable> parent)
{
    parent_ = std::move(parent);
}

void Variable::set_value(ValuePtr value)
{
    if (is_temporary() or is_local())
    {
        value_ = std::move(value);
    }
    else
    {
        Runtime::get().serializer().write_node_def_output(shared_from_this(), value);
    }

    is_initialized_ = true;
}

void Variable::copy_children(const vector<VarPtr>& children)
{
    children_.clear();
    for (size_t i = 0; i < children.size(); ++i)
    {
        VarPtr child = create(type_->field(i).modifiers(), type_->field_type(i), children[i]);
        child->set_parent(weak_from_this());
        if (not name_.empty())
            child->set_name(get_port_name(name_, i));
        children_.push_back(std::move(child));
    }

    is_initialized_ = true;
}
