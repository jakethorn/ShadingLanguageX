//
// Created by jaket on 16/04/2026.
//

#include "Variable.h"

#include <cassert>

#include "Runtime.h"
#include "Scope.h"
#include "expressions/ExpressionFactory.h"
#include "mtlx/mtlx_utils.h"
#include "values/NodeValue.h"

Variable::Variable(ModifierList mods, TypeInfoPtr type) : type_{std::move(type)}
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

const TypeInfoPtr& Variable::type() const
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
        children_[i]->set_name(port_name(name_, i));
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
        const VarPtr self = shared_from_this();
        return Runtime::get().serializer().write_node_def_input(self);
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

void Variable::set_as_node_input(const mx::NodePtr& node, const string& input_name)
{
    if (has_value())
    {
        value()->set_as_node_input(node, input_name);
    }
    else
    {
        for (size_t i = 0; i < children_.size(); ++i)
        {
            children_[i]->set_as_node_input(node, port_name(input_name, i));
        }
    }
}

void Variable::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name)
{
    if (has_value())
    {
        value()->set_as_node_graph_output(node_graph, output_name);
    }
    else
    {
        for (size_t i = 0; i < children_.size(); ++i)
        {
            children_[i]->set_as_node_graph_output(node_graph, port_name(output_name, i));
        }
    }
}

void Variable::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name)
{
    if (has_value())
    {
        value()->set_as_node_def_input(node_def, input_name);
    }
    else
    {
        for (size_t i = 0; i < children_.size(); ++i)
        {
            children_[i]->set_as_node_def_input(node_def, port_name(input_name, i));
        }
    }
}

void Variable::add_to_scope(string name)
{
    const VarPtr self = shared_from_this();
    Runtime::get().scope().add_variable(std::move(name), self);
}

bool Variable::is_local()
{
    const VarPtr self = shared_from_this();
    return Runtime::get().scope().is_variable_local(self);
}

string Variable::str() const
{
    return name_;
}

VarPtr Variable::create(ModifierList mods, TypeInfoPtr type, const vector<VarPtr>& children)
{
    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->copy_children(children);
    return var;
}

VarPtr Variable::create(ModifierList mods, TypeInfoPtr type, ValuePtr value)
{
    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->set_value(std::move(value));
    return var;
}

VarPtr Variable::create(ModifierList mods, TypeInfoPtr type, const VarPtr& value)
{
    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->copy_value(value);
    return var;
}

VarPtr Variable::create(TypeInfoPtr type, const vector<VarPtr>& children)
{
    return create(ModifierList{}, std::move(type), children);
}

VarPtr Variable::create(TypeInfoPtr type, ValuePtr value)
{
    return create(ModifierList{}, std::move(type), std::move(value));
}

VarPtr Variable::create(TypeInfoPtr type, const VarPtr& value)
{
    return create(ModifierList{}, std::move(type), value);
}

VarPtr Variable::create(ValuePtr value)
{
    TypeInfoPtr type = value->type();
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
        const VarPtr self = shared_from_this();
        Runtime::get().serializer().write_node_def_output(self, value);
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
            child->set_name(port_name(name_, i));
        children_.push_back(std::move(child));
    }

    is_initialized_ = true;
}
