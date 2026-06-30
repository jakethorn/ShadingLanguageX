//
// Created by jaket on 16/04/2026.
//

#include "Variable.h"

#include "CompileError.h"
#include "Scope.h"
#include "Type.h"
#include "mtlx/MtlXSerializer.h"
#include "mtlx/mtlx_utils.h"
#include "utils/str_utils.h"
#include "values/NodeOutputValue.h"
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

bool Variable::is_global() const
{
    return mods_.contains(TokenType::Global);
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
        children_[i]->set_name(name_, type(), i);
    }

    set_node_name(name_);
}

void Variable::set_name(const string& name, const TypePtr& parent_type, const size_t index)
{
    name_ = get_port_name(name, index);
    for (size_t i = 0; i < children_.size(); ++i)
    {
        children_[i]->set_name(name_, type(), i);
    }

    const Field& field = parent_type->field(index);
    const string child_name = field.has_name() ? field.name() : ::str(index);
    set_node_name(name + "__" + child_name);
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

bool Variable::is_local()
{
    return scope().is_variable_local(shared_from_this());
}

bool Variable::has_parent() const
{
    return parent_.lock() != nullptr;
}

VarPtr Variable::parent() const
{
    return parent_.lock();
}

const vector<VarPtr>& Variable::children() const
{
    return children_;
}

size_t Variable::child_count() const
{
    return children_.size();
}

VarPtr Variable::child(const size_t index)
{
    if (index >= children_.size())
        throw CompileError{"Index out of bounds: " + ::str(index)};

    return children_.at(index);
}

VarPtr Variable::child(const string& field_name)
{
    return child(type_->field_index(field_name));
}

VarPtr Variable::oldest()
{
    if (has_parent())
        return parent()->oldest();
    else
        return shared_from_this();
}

bool Variable::has_value() const
{
    return value_impl() != nullptr;
}

ValuePtr Variable::value()
{
    if (is_temporary() or is_local())
    {
        return value_impl();
    }
    else
    {
        return serializer().write_node_def_input(shared_from_this());
    }
}

ValuePtr Variable::raw_value() const
{
    return value_impl();
}

VarPtr Variable::copy()
{
    return create(shared_from_this());
}

void Variable::copy(const VarPtr& other)
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
        copy_value(other->value());
    }
    else
    {
        copy_children(other->children_);
    }
}

bool Variable::equals(const VarPtr& other) const
{
    if (has_value() != other->has_value())
        return false;

    if (has_value())
    {
        return value_impl()->equals(other->value_impl());
    }
    else
    {
        if (child_count() != other->child_count())
            return false;

        for (size_t i = 0; i < child_count(); i++)
        {
            if (not children_[i]->equals(other->children_[i]))
                return false;
        }

        return true;
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

Scope& Variable::defining_scope()
{
    return scope().get_defining_scope(shared_from_this());
}

void Variable::add_to_scope(string name)
{
    scope().add_variable(name, shared_from_this());

    if (name == "this"s)
    {
        // also bring in children
        for (size_t i = 0; i < child_count(); ++i)
        {
            const string& child_name = type()->field_name(i);
            if (scope().has_variable(child_name))
                continue;
            child(i)->add_to_scope(child_name);
        }
    }
}

string Variable::str() const
{
    if (has_value())
    {
        return value_impl()->str();
    }
    else
    {
        string result = "{";
        for (size_t i = 0; i < children_.size(); ++i)
        {
            const string field_name = type_->field(i).has_name() ? type_->field_name(i) : "field_" + ::str(i);
            const VarPtr& child = children_[i];
            result += "\n\t" + field_name + ": " + child->str();
        }
        return result + "\n}";
    }
}

VarPtr Variable::create(ModifierList mods, TypePtr type, const vector<VarPtr>& children)
{
    if (contains_auto(type))
        type = remove_auto(type, type_of(children));

    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->copy_children(children);
    return var;
}

VarPtr Variable::create(ModifierList mods, TypePtr type, const vector<primitive_t>& children)
{
    vector<VarPtr> vars;
    vars.reserve(children.size());
    for (const primitive_t& child : children)
        vars.push_back(create(child));

    return create(std::move(mods), std::move(type), vars);
}

VarPtr Variable::create(ModifierList mods, TypePtr type, ValuePtr value)
{
    if (contains_auto(type))
        type = remove_auto(type, value->type());

    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->copy_value(std::move(value));
    return var;
}

VarPtr Variable::create(ModifierList mods, TypePtr type, const VarPtr& value)
{
    if (contains_auto(type))
        type = remove_auto(type, value->type());

    VarPtr var = std::make_shared<Variable>(std::move(mods), std::move(type));
    var->copy(value);
    return var;
}

VarPtr Variable::create(ModifierList mods, ValuePtr value)
{
    TypePtr type = value->type();
    return create(std::move(mods), std::move(type), std::move(value));
}

VarPtr Variable::create(ModifierList mods, primitive_t value)
{
    ValuePtr basic_value = std::make_shared<BasicValue>(std::move(value));
    return create(std::move(mods), std::move(basic_value));
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

VarPtr Variable::create(const vector<VarPtr>& children)
{
    return create(ModifierList{}, type_of(children), children);
}

VarPtr Variable::create(const vector<primitive_t>& children)
{
    vector<VarPtr> vars;
    vars.reserve(children.size());
    for (const primitive_t& child : children)
        vars.push_back(create(child));

    return create(vars);
}

VarPtr Variable::create(ValuePtr value)
{
    TypePtr type = value->type();
    return create(ModifierList{}, std::move(type), std::move(value));
}

VarPtr Variable::create(primitive_t value)
{
    ValuePtr basic_value = std::make_shared<BasicValue>(std::move(value));
    return create(std::move(basic_value));
}

VarPtr Variable::create(const VarPtr& value)
{
    return create(ModifierList{}, value->type(), value);
}

VarPtr Variable::create(const mxslc::Variable& var)
{
    if (var.has_value())
        return create(var.value());
    TypePtr type = scope().resolve_type(Type::of(var));
    vector<VarPtr> children;
    children.reserve(var.children().size());
    for (const mxslc::VariablePtr& child : var.children())
        children.emplace_back(create(*child));
    return create(std::move(type), children);
}

void Variable::set_node_name(const string& name) const
{
    if (const shared_ptr<NodeValue> node_value = std::dynamic_pointer_cast<NodeValue>(value_impl()))
    {
        node_value->set_node_name(name);
        return;
    }

    mx::NodePtr node;
    for (const VarPtr& child : children_)
    {
        ValuePtr value = child->value_impl();
        if (const shared_ptr<NodeOutputValue> output_value = std::dynamic_pointer_cast<NodeOutputValue>(value))
        {
            if (node == nullptr)
            {
                node = output_value->node();
            }
            else
            {
                if (output_value->node() != node)
                    return;
            }
        }
    }

    for (const VarPtr& child : children_)
    {
        ValuePtr value = child->value_impl();
        if (const shared_ptr<NodeOutputValue> output_value = std::dynamic_pointer_cast<NodeOutputValue>(value))
        {
            output_value->set_node_name(name);
        }
    }
}

void Variable::copy_value(ValuePtr value)
{
    if (is_temporary() or is_local())
    {
        copy_value_impl(std::move(value));
    }
    else
    {
        serializer().write_node_def_output(shared_from_this(), value);
    }

    is_initialized_ = true;
}

void Variable::copy_children(const vector<VarPtr>& children)
{
    children_.clear();
    for (size_t i = 0; i < children.size(); ++i)
    {
        VarPtr child = create(type_->field(i).modifiers(), type_->field_type(i), children[i]);
        child->parent_ = weak_from_this();
        if (not name_.empty())
            child->set_name(get_port_name(name_, i));
        children_.push_back(std::move(child));
    }

    is_initialized_ = true;
}

TypePtr Variable::type_of(const vector<VarPtr>& children)
{
    vector<TypePtr> fields;
    fields.reserve(children.size());
    for (const VarPtr& child : children)
        fields.push_back(child->type());

    return scope().resolve_type(
        std::make_shared<Type>(std::move(fields))
    );
}

bool Variable::contains_auto(const TypePtr& type)
{
    if (type->is_auto())
        return true;
    if (type->has_fields())
    {
        for (const Field& field : type->fields())
        {
            if (contains_auto(field.type()))
                return true;
        }
    }
    return false;
}

TypePtr Variable::remove_auto(const TypePtr& original_type, const TypePtr& value_type)
{
    if (original_type->is_auto())
        return value_type;
    if (original_type->has_fields())
    {
        vector<Field> fields;
        fields.reserve(original_type->field_count());
        for (size_t i = 0; i < original_type->field_count(); ++i)
        {
            Field original_field = original_type->field(i);
            fields.emplace_back(
                original_field.modifiers(),
                remove_auto(original_field.type(), value_type->field_type(i)),
                original_field.name()
            );
        }
        return std::make_shared<Type>(original_type->name(), std::move(fields));
    }
    return original_type;
}
