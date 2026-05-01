//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_VARIABLE_H
#define MXSLC_VARIABLE_H

#include "CompileError.h"
#include "ModifierList.h"
#include "utils/common.h"
#include "values/BasicValue.h"

class Variable : public std::enable_shared_from_this<Variable>
{
public:
    Variable(ModifierList mods, TypeInfoPtr type);

    bool is_const() const;
    bool is_mutable() const;
    const ModifierList& modifiers() const;
    void set_modifiers(ModifierList mods);

    const TypeInfoPtr& type() const;

    const string& name() const;
    void set_name(string name);

    bool is_assignable() const;
    bool is_temporary() const;

    bool has_parent() const;
    VarPtr parent() const;
    size_t child_count() const;
    VarPtr child(size_t index);
    VarPtr child(const string& field_name);

    bool has_value() const;
    ValuePtr value();
    void copy_value(const VarPtr& other);

    void uninitialize();

    void set_as_node_input(const mx::NodePtr& node, const string& input_name);
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name);
    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name);

    void add_to_scope(string name);
    bool is_local();

    string str() const;

    template<typename T>
    T value_as() const
    {
        if (const shared_ptr<BasicValue>& value = std::dynamic_pointer_cast<BasicValue>(value_))
            return value->get<T>();
        else
            throw CompileError{"Value is not a compile-time "s + typeid(T).name()};
    }

    static VarPtr create(ModifierList mods, TypeInfoPtr type, const vector<VarPtr>& children);
    static VarPtr create(ModifierList mods, TypeInfoPtr type, ValuePtr value);
    static VarPtr create(ModifierList mods, TypeInfoPtr type, const VarPtr& value);
    static VarPtr create(TypeInfoPtr type, const vector<VarPtr>& children);
    static VarPtr create(TypeInfoPtr type, ValuePtr value);
    static VarPtr create(TypeInfoPtr type, const VarPtr& value);
    static VarPtr create(ValuePtr value);
    static VarPtr create(const VarPtr& value);

private:
    void set_parent(weak_ptr<Variable> parent);
    void set_value(ValuePtr value);
    void copy_children(const vector<VarPtr>& children);

    ModifierList mods_;
    TypeInfoPtr type_;
    weak_ptr<Variable> parent_;
    vector<VarPtr> children_;
    ValuePtr value_;
    string name_;
    bool is_initialized_ = false;
};

#endif //MXSLC_VARIABLE_H
