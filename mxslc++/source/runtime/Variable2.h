//
// Created by jaket on 16/04/2026.
//

#ifndef MXSLC_VARIABLE2_H
#define MXSLC_VARIABLE2_H

#include "CompileError.h"
#include "ModifierList.h"
#include "utils/common.h"
#include "values/BasicValue.h"

class Variable2
{
public:
    Variable2(ModifierList mods, TypeInfoPtr type, string name, vector<VarPtr2> children);
    Variable2(ModifierList mods, TypeInfoPtr type, string name, ValuePtr value);
    Variable2(ModifierList mods, TypeInfoPtr type, string name, const VarPtr2& value);
    Variable2(TypeInfoPtr type, vector<VarPtr2> children);
    explicit Variable2(ValuePtr value);

    void copy_value(const VarPtr2& value);

    bool is_const() const;
    bool is_mutable() const;

    const TypeInfoPtr& type() const;
    const string& name() const;

    size_t child_count() const;
    VarPtr2 child(size_t index);
    VarPtr2 child(const string& name);

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
    string name_;
    vector<VarPtr2> children_;
    ValuePtr value_;
};

#endif //MXSLC_VARIABLE2_H
