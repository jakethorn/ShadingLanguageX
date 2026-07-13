//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_VARIABLE_H
#define MXSLC_VARIABLE_H

#include "common.h"
#include "primitive/Primitive.h"
#include "runtime/ModifierList.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::runtime
{
    class VariableFactory;

    class Variable : public std::enable_shared_from_this<Variable>, protected runtime_utils::RuntimeAware
    {
        friend class VariableFactory;

    public:
        Variable(ModifierList mods, TypePtr type);
        ~Variable() override = default;

        bool is_const() const;
        bool is_mutable() const;
        bool is_global() const;
        const ModifierList& modifiers() const;
        void set_modifiers(ModifierList mods);

        const TypePtr& type() const;

        const string& name() const;
        void set_name(string name);

        bool is_assignable() const;
        bool is_temporary() const;
        bool is_local();

        bool has_parent() const;
        VarPtr parent() const;
        VarPtr oldest_ancestor();

        size_t child_count() const;
        const vector<VarPtr>& children() const;
        VarPtr child(size_t index);
        VarPtr child(const string& field_name);

        bool has_value() const;
        ValuePtr value();
        ValuePtr raw_value() const;

        VarPtr copy();
        void copy(const VarPtr& other);
        bool equals(const VarPtr& other) const;

        void uninitialize();

        Scope& defining_scope();
        void add_to_scope(string name);

        bool is_basic() const;
        const Primitive& basic() const;

        template<typename T>
        T basic() const { return basic().as<T>(); }

        string str() const;

    protected:
        virtual ValuePtr value_impl() const { return value_; }
        virtual void copy_value_impl(ValuePtr value) { value_ = std::move(value); }
        virtual void set_node_name(const string& name) const;

    private:
        void copy_value(ValuePtr value);
        void copy_children(const vector<VarPtr>& children);
        void set_name(const string& name, const TypePtr& parent_type, size_t index);

        ModifierList mods_;
        TypePtr type_;
        weak_ptr<Variable> parent_;
        vector<VarPtr> children_;
        ValuePtr value_;
        string name_;
        bool is_initialized_{false};
    };
}

#endif //MXSLC_VARIABLE_H
