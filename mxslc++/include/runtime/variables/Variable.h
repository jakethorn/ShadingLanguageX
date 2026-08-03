//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_VARIABLE_H
#define MXSLC_VARIABLE_H

#include "common.h"
#include "Primitive.h"
#include "runtime/ModifierList.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::runtime
{
    using runtime_utils::RuntimeAware;

    class Variable : public std::enable_shared_from_this<Variable>, protected RuntimeAware, public Stringable
    {
        friend class Scope;
        friend class VariableFactory;

    public:
        Variable(ModifierList mods, TypePtr type);
        ~Variable() override = default;

        bool is_comptime() const;
        bool is_const() const;
        bool is_mutable() const;
        const ModifierList& modifiers() const;
        void set_modifiers(ModifierList mods);

        const TypePtr& type() const;

        const string& name() const;
        void disable_node_naming() { can_name_nodes_ = false; }

        virtual bool is_assignable() const;
        virtual bool is_temporary() const;
        virtual bool is_local();

        bool has_parent() const;
        VarPtr parent() const;
        VarPtr oldest_ancestor();

        size_t child_count() const;
        bool has_children() const;
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

        Scope* defining_scope() const;
        void add_to_scope(string name);

        template<typename T>
        bool is_compile_time() const { return is_compile_time() and has_value() and compile_time_value().is_a<T>(); }
        bool is_compile_time() const;

        template<typename T>
        T compile_time_value() const { return compile_time_value().as<T>(); }
        Primitive compile_time_value() const;

        // Value comes from an external source, e.g., a global or entry function argument
        bool is_external() const { return is_external_; }
        void set_is_external();

        string to_string() const override;

    protected:
        virtual ValuePtr value_impl() const { return value_; }
        virtual void copy_value_impl(ValuePtr value) { value_ = std::move(value); }
        virtual void set_node_name(const string& name) const;

    private:
        void copy_value(ValuePtr value);
        void copy_children(const vector<VarPtr>& children);

        void set_name(string name);
        void set_name(const string& name, const TypePtr& parent_type, size_t index);

        ModifierList mods_;
        TypePtr type_;
        weak_ptr<Variable> parent_;
        vector<VarPtr> children_;
        ValuePtr value_;
        string name_;
        Scope* defining_scope_{nullptr};
        bool can_name_nodes_{true};
        bool is_initialized_{false};
        bool is_external_{false};
    };
}

#endif //MXSLC_VARIABLE_H
