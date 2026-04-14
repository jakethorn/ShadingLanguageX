//
// Created by jaket on 18/01/2026.
//

#ifndef MXSLC_FIELDINFO_H
#define MXSLC_FIELDINFO_H

#include "utils/common.h"
#include "ModifierList.h"

class FieldInfo
{
    friend class Scope;

public:
    FieldInfo(ModifierList mods, TypeInfoPtr type, string name, ExprPtr initializer)
        : mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, initializer_{std::move(initializer)}
    {
        mods_.validate(TokenType::Const, TokenType::Mutable);
    }

    explicit FieldInfo(TypeInfoPtr type) : FieldInfo{ModifierList{}, std::move(type), ""s, nullptr} { }

    const ModifierList& modifiers() const { return mods_; }
    bool is_const() const { return mods_.contains(TokenType::Const); }
    bool is_mutable() const { return mods_.contains(TokenType::Mutable); }
    TypeInfoPtr type() const { return type_; }
    bool has_name() const { return not name_.empty(); }
    const string& name() const { return name_; }
    ExprPtr initializer() const { return initializer_; }

    FieldInfo instantiate_template_types(const TypeInfoPtr& template_type) const;
    void set_const(bool cnt) const;
    void set_mutable(bool mut) const;
    string str() const;

private:
    mutable ModifierList mods_;
    TypeInfoPtr type_;
    string name_;
    ExprPtr initializer_;
};

#endif //MXSLC_FIELDINFO_H
