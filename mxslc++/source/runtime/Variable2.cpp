//
// Created by jaket on 16/04/2026.
//

#include "Variable2.h"

#include <cassert>

#include "Runtime.h"
#include "Scope.h"
#include "expressions/ExpressionFactory.h"
#include "mtlx/mtlx_utils.h"
#include "values/NodeValue.h"

Variable2::Variable2(ModifierList mods, TypeInfoPtr type) : type_{std::move(type)}
{
    set_modifiers(std::move(mods));
}

bool Variable2::is_const() const
{
    return mods_.contains(TokenType::Const);
}

bool Variable2::is_mutable() const
{
    return mods_.contains(TokenType::Mutable);
}

const ModifierList& Variable2::modifiers() const
{
    return mods_;
}

void Variable2::set_modifiers(ModifierList mods)
{
    mods_ = std::move(mods);
    mods_.validate(TokenType::Const, TokenType::Mutable, TokenType::Global);
    if (is_const() and is_mutable())
        throw CompileError{"Variables cannot be both const and mutable"s};
}

const TypeInfoPtr& Variable2::type() const
{
    return type_;
}

const string& Variable2::name() const
{
    return name_;
}

void Variable2::set_name(string name)
{
    name_ = std::move(name);
    for (size_t i = 0; i < children_.size(); ++i)
    {
        children_[i]->set_name(port_name(name_, i));
    }
}

bool Variable2::is_assignable() const
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

bool Variable2::is_temporary() const
{
    if (has_parent())
        return parent()->is_temporary();
    return name_.empty();
}

bool Variable2::has_parent() const
{
    return parent_.lock() != nullptr;
}

VarPtr2 Variable2::parent() const
{
    return parent_.lock();
}

size_t Variable2::child_count() const
{
    return children_.size();
}

VarPtr2 Variable2::child(const size_t index)
{
    return children_.at(index);
}

VarPtr2 Variable2::child(const string& field_name)
{
    return child(type_->field_index(field_name));
}

bool Variable2::has_value() const
{
    return value_ != nullptr;
}

ValuePtr Variable2::value()
{
    if (is_temporary() or is_local())
    {
        return value_;
    }
    else
    {
        const VarPtr2 self = shared_from_this();
        return Runtime::get().serializer().write_node_def_input(self);
    }
}

void Variable2::copy_value(const VarPtr2& other)
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

void Variable2::uninitialize()
{
    is_initialized_ = false;
    for (const VarPtr2& child : children_)
    {
        child->uninitialize();
    }
}

void Variable2::set_as_node_input(const mx::NodePtr& node, const string& input_name)
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

void Variable2::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name)
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

void Variable2::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name)
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

void Variable2::add_to_scope()
{
    assert(not name_.empty());
    add_to_scope(name_);
}

void Variable2::add_to_scope(string name)
{
    const VarPtr2 self = shared_from_this();
    Runtime::get().scope().add_variable(std::move(name), self);
}

bool Variable2::is_local()
{
    const VarPtr2 self = shared_from_this();
    return Runtime::get().scope().is_variable_local(self);
}

string Variable2::str() const
{
    return name_;
}

VarPtr2 Variable2::create(ModifierList mods, TypeInfoPtr type, const vector<VarPtr2>& children)
{
    VarPtr2 var = std::make_shared<Variable2>(std::move(mods), std::move(type));
    var->copy_children(children);
    return var;
}

VarPtr2 Variable2::create(ModifierList mods, TypeInfoPtr type, ValuePtr value)
{
    VarPtr2 var = std::make_shared<Variable2>(std::move(mods), std::move(type));
    var->set_value(std::move(value));
    return var;
}

VarPtr2 Variable2::create(ModifierList mods, TypeInfoPtr type, const VarPtr2& value)
{
    VarPtr2 var = std::make_shared<Variable2>(std::move(mods), std::move(type));
    var->copy_value(value);
    return var;
}

VarPtr2 Variable2::create(TypeInfoPtr type, const vector<VarPtr2>& children)
{
    return create(ModifierList{}, std::move(type), children);
}

VarPtr2 Variable2::create(TypeInfoPtr type, ValuePtr value)
{
    return create(ModifierList{}, std::move(type), std::move(value));
}

VarPtr2 Variable2::create(TypeInfoPtr type, const VarPtr2& value)
{
    return create(ModifierList{}, std::move(type), value);
}

VarPtr2 Variable2::create(ValuePtr value)
{
    TypeInfoPtr type = value->type();
    return create(ModifierList{}, std::move(type), std::move(value));
}

VarPtr2 Variable2::create(const VarPtr2& value)
{
    return create(ModifierList{}, value->type(), value);
}

void Variable2::set_parent(weak_ptr<Variable2> parent)
{
    parent_ = std::move(parent);
}

void Variable2::set_value(ValuePtr value)
{
    if (is_temporary() or is_local())
    {
        value_ = std::move(value);
    }
    else
    {
        const VarPtr2 self = shared_from_this();
        Runtime::get().serializer().write_node_def_output(self, value);
    }

    is_initialized_ = true;
}

void Variable2::copy_children(const vector<VarPtr2>& children)
{
    children_.clear();
    for (size_t i = 0; i < children.size(); ++i)
    {
        VarPtr2 child = create(type_->field(i).modifiers(), type_->field_type(i), children[i]);
        child->set_parent(weak_from_this());
        if (not name_.empty())
            child->set_name(port_name(name_, i));
        children_.push_back(std::move(child));
    }

    is_initialized_ = true;
}
