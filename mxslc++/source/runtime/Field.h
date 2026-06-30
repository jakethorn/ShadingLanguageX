//
// Created by jaket on 18/01/2026.
//

#ifndef MXSLC_FIELD_H
#define MXSLC_FIELD_H

#include "utils/common.h"
#include "ModifierList.h"

class Field
{
    friend class Scope;

public:
    Field(TypePtr type, string name)
        : type_{std::move(type)}, name_{std::move(name)}
    {

    }

    Field(ModifierList mods, TypePtr type, string name, const bool can_be_global = false)
        : mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}
    {
        if (can_be_global)
            mods_.validate(TokenType::Const, TokenType::Mutable, TokenType::Global);
        else
            mods_.validate(TokenType::Const, TokenType::Mutable);
    }

    explicit Field(TypePtr type) : Field{ModifierList{}, std::move(type), ""s} { }

    const ModifierList& modifiers() const { return mods_; }
    bool is_const() const { return mods_.contains(TokenType::Const); }
    bool is_mutable() const { return mods_.contains(TokenType::Mutable); }
    bool is_global() const { return mods_.contains(TokenType::Global); }
    TypePtr type() const { return type_; }
    bool has_name() const { return not name_.empty(); }
    const string& name() const { return name_; }

    Field instantiate_template_types(const TypePtr& template_type) const;

    string str() const;

private:
    ModifierList mods_;
    TypePtr type_;
    string name_;
};

#endif //MXSLC_FIELD_H
