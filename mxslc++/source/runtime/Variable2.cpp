//
// Created by jaket on 16/04/2026.
//

#include "Variable2.h"

#include <cassert>

#include "Runtime.h"
#include "Scope.h"
#include "expressions/ExpressionFactory.h"
#include "mtlx/mtlx_utils.h"

Variable2::Variable2(ModifierList mods, TypeInfoPtr type, vector<VarPtr2> children) : type_{std::move(type)}, children_{std::move(children)}
{
    set_modifiers(std::move(mods));
}

Variable2::Variable2(ModifierList mods, TypeInfoPtr type, ValuePtr value) : type_{std::move(type)}, value_{std::move(value)}
{
    set_modifiers(std::move(mods));
}

Variable2::Variable2(ModifierList mods, TypeInfoPtr type, const VarPtr2& value) : type_{std::move(type)}
{
    set_modifiers(std::move(mods));
    copy_value(value);
}

Variable2::Variable2(TypeInfoPtr type, const VarPtr2& value) : type_{std::move(type)}
{
    copy_value(value);
}

Variable2::Variable2(TypeInfoPtr type, vector<VarPtr2> children) : type_{std::move(type)}, children_{std::move(children)}
{

}

Variable2::Variable2(ValuePtr value) : type_{value->type()}, value_{std::move(value)}
{

}

void Variable2::copy_value(const VarPtr2& other)
{
    if (other->has_value())
    {
        set_value(other->value());
    }
    else
    {
        for (size_t i = 0; i < other->child_count(); i++)
        {
            VarPtr2 child = std::make_shared<Variable2>(type_->field_type(i), other->child(i));
            children_.push_back(std::move(child));
        }
    }
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
    return children_.at(type_->field_index(field_name));
}

bool Variable2::has_value() const
{
    return value_ != nullptr;
}

ValuePtr Variable2::value()
{
    if (is_nonlocal())
    {
        const VarPtr2 self = shared_from_this();
        return Runtime::get().serializer().write_node_def_input(self);
    }
    else
    {
        return value_;
    }
}

void Variable2::set_value(ValuePtr value)
{
    if (is_nonlocal())
    {
        const VarPtr2 self = shared_from_this();
        Runtime::get().serializer().write_node_def_output(self, value);
    }
    else
    {
        value_ = std::move(value);
    }
}

void Variable2::set_as_node_input(const mx::NodePtr& node, const string& input_name) const
{
    if (has_value())
    {
        value_->set_as_node_input(node, input_name);
    }
    else
    {
        for (size_t i = 0; i < children_.size(); ++i)
        {
            children_[i]->set_as_node_input(node, port_name(input_name, i));
        }
    }
}

void Variable2::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    if (has_value())
    {
        value_->set_as_node_graph_output(node_graph, output_name);
    }
    else
    {
        for (size_t i = 0; i < children_.size(); ++i)
        {
            children_[i]->set_as_node_graph_output(node_graph, port_name(output_name, i));
        }
    }
}

void Variable2::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
{
    if (has_value())
    {
        value_->set_as_node_def_input(node_def, input_name);
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

bool Variable2::is_nonlocal() const
{
    return Runtime::get().scope().is_variable_nonlocal(name_);
}

string Variable2::str() const
{
    return name_;
}
