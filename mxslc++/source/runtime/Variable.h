//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_VARIABLE_H
#define MXSLC_VARIABLE_H

#include "utils/common.h"
#include "ModifierList.h"
#include "values/BasicValue.h"

class Variable : public std::enable_shared_from_this<Variable>
{
public:
    Variable(ModifierList mods, TypePtr type);
    virtual ~Variable() = default;

    bool is_const() const;
    bool is_mutable() const;
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
    size_t child_count() const;
    VarPtr child(size_t index);
    VarPtr child(const string& field_name);

    bool has_value() const;
    ValuePtr value();
    ValuePtr raw_value() const;
    void copy_value(const VarPtr& other);

    void uninitialize();

    Scope& defining_scope();
    void add_to_scope(string name);

    string str() const;

    template<typename T>
    T value_as() const
    {
        if (const shared_ptr<BasicValue>& value = std::dynamic_pointer_cast<BasicValue>(value_))
            return value->get<T>();
        throw_error("Value is not a compile-time "s + typeid(T).name());
        return T{};
    }

    static VarPtr create(ModifierList mods, TypePtr type, const vector<VarPtr>& children);
    static VarPtr create(ModifierList mods, TypePtr type, ValuePtr value);
    static VarPtr create(ModifierList mods, TypePtr type, const VarPtr& value);
    static VarPtr create(TypePtr type, const vector<VarPtr>& children);
    static VarPtr create(TypePtr type, ValuePtr value);
    static VarPtr create(TypePtr type, const VarPtr& value);
    static VarPtr create(ValuePtr value);
    static VarPtr create(const VarPtr& value);

protected:
    virtual ValuePtr value_impl() const { return value_; }
    virtual void set_value_impl(ValuePtr value) { value_ = std::move(value); }

private:
    void set_parent(weak_ptr<Variable> parent);
    void set_value(ValuePtr value);
    void copy_children(const vector<VarPtr>& children);

    static void throw_error(const string& message);

    ModifierList mods_;
    TypePtr type_;
    weak_ptr<Variable> parent_;
    vector<VarPtr> children_;
    ValuePtr value_;
    string name_;
    bool is_initialized_ = false;
};

#endif //MXSLC_VARIABLE_H
