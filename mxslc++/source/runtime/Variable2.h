//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_VARIABLE2_H
#define MXSLC_VARIABLE2_H

#include "CompileError.h"
#include "ModifierList.h"
#include "utils/common.h"
#include "values/BasicValue.h"

class Variable2 : public std::enable_shared_from_this<Variable2>
{
public:
    Variable2(ModifierList mods, TypeInfoPtr type, vector<VarPtr2> children);
    Variable2(ModifierList mods, TypeInfoPtr type, ValuePtr value);
    Variable2(ModifierList mods, TypeInfoPtr type, const VarPtr2& value);
    Variable2(TypeInfoPtr type, vector<VarPtr2> children);
    Variable2(TypeInfoPtr type, const VarPtr2& value);
    explicit Variable2(ValuePtr value);

    void copy_value(const VarPtr2& other);

    bool is_const() const;
    bool is_mutable() const;
    const ModifierList& modifiers() const;
    void set_modifiers(ModifierList mods);

    const TypeInfoPtr& type() const;

    const string& name() const;
    void set_name(string name);

    size_t child_count() const;
    VarPtr2 child(size_t index);
    VarPtr2 child(const string& field_name);

    bool has_value() const;
    ValuePtr value();
    void set_value(ValuePtr value);
    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const;
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const;
    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const;

    void add_to_scope();
    void add_to_scope(string name);
    bool is_nonlocal() const;

    string str() const;

    template<typename T>
    T value_as() const
    {
        if (const shared_ptr<BasicValue>& value = std::dynamic_pointer_cast<BasicValue>(value_))
            return value->get<T>();
        else
            throw CompileError{"Value is not a compile-time "s + typeid(T).name()};
    }

private:
    ModifierList mods_;
    TypeInfoPtr type_;
    vector<VarPtr2> children_;
    ValuePtr value_;
    string name_;
};

#endif //MXSLC_VARIABLE2_H
