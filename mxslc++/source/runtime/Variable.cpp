//
// Created by jaket on 16/04/2026.
//

#include "runtime/Variable.h"

#include "runtime/interface.h"
#include "runtime/Scope.h"
#include "runtime/Type.h"
#include "serialize/Serializer.h"
#include "serialize/values/interface.h"
#include "serialize/values/NodeOutputValue.h"
#include "serialize/values/NodeValue.h"
#include "utils/mtlx_utils.h"
#include "utils/string_utils.h"
#include "errors/CompileError.h"
#include "serialize/serializer_utils.h"

namespace mxslc::runtime
{
    using mtlx_utils::get_port_name;

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
            throw CompileError{"Variables cannot be both const and mutable"};
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
            children_[i]->set_name(name_, type_, i);
        }

        set_node_name(name_);
    }

    void Variable::set_name(const string& name, const TypePtr& parent_type, const size_t index)
    {
        name_ = get_port_name(name, index);
        for (size_t i = 0; i < children_.size(); ++i)
        {
            children_[i]->set_name(name_, type_, i);
        }

        const Field& field = parent_type->field(index);
        const string child_name = field.has_name() ? field.name() : string_utils::str(index);
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

    VarPtr Variable::oldest_ancestor()
    {
        if (has_parent())
            return parent()->oldest_ancestor();
        else
            return shared_from_this();
    }

    size_t Variable::child_count() const
    {
        return children_.size();
    }

    const vector<VarPtr>& Variable::children() const
    {
        return children_;
    }

    VarPtr Variable::child(const size_t index)
    {
        if (index >= children_.size())
            throw CompileError{"Index out of bounds: " + string_utils::str(index)};

        return children_.at(index);
    }

    VarPtr Variable::child(const string& field_name)
    {
        return child(type_->field_index(field_name));
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
        return create_variable(shared_from_this());
    }

    void Variable::copy(const VarPtr& other)
    {
        if (is_initialized_)
        {
            if (is_temporary())
                throw CompileError{"Cannot assign value to temporary variable"};
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

        if (name == "this")
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
                const string field_name = type_->field(i).has_name() ? type_->field_name(i) : "field_" + string_utils::str(i);
                const VarPtr& child = children_[i];
                result += "\n\t" + field_name + ": " + child->str();
            }
            return result + "\n}";
        }
    }

    void Variable::set_node_name(const string& name) const
    {
        if (const NodeValuePtr node_value = cast_value<NodeValue>(value_impl()))
        {
            node_value->set_node_name(name);
            return;
        }

        mx::NodePtr node;
        for (const VarPtr& child : children_)
        {
            ValuePtr value = child->value_impl();
            if (const NodeOutputValuePtr output_value = cast_value<NodeOutputValue>(value))
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
            if (const NodeOutputValuePtr output_value = cast_value<NodeOutputValue>(value))
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
            VarPtr child = create_variable(type_->field(i).modifiers(), type_->field_type(i), children[i]);
            child->parent_ = weak_from_this();
            if (not name_.empty())
                child->set_name(get_port_name(name_, i));
            children_.push_back(std::move(child));
        }

        is_initialized_ = true;
    }
}
