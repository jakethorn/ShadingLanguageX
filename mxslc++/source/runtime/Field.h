//
// Created by jaket on 18/01/2026.
//

#ifndef MXSLC_FIELDINFO_H
#define MXSLC_FIELDINFO_H

#include "utils/common.h"
#include "ModifierList.h"

class Field
{
    friend class Scope;

public:
    Field(ModifierList mods, TypePtr type, string name)
        : mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}
    {
        mods_.validate(TokenType::Const, TokenType::Mutable);
    }

    explicit Field(TypePtr type) : Field{ModifierList{}, std::move(type), ""s} { }

    const ModifierList& modifiers() const { return mods_; }
    bool is_const() const { return mods_.contains(TokenType::Const); }
    bool is_mutable() const { return mods_.contains(TokenType::Mutable); }
    TypePtr type() const { return type_; }
    bool has_name() const { return not name_.empty(); }
    const string& name() const { return name_; }
    ExprPtr initializer() const { return initializer_; }

    Field instantiate_template_types(const TypePtr& template_type) const;

    string str() const;

private:
    ModifierList mods_;
    TypePtr type_;
    string name_;
};

#endif //MXSLC_FIELDINFO_H
