//
// Created by jaket on 18/01/2026.
//

#ifndef MXSLC_FIELD_H
#define MXSLC_FIELD_H

#include "common.h"
#include "TokenType.h"
#include "runtime/ModifierList.h"
#include "runtime/utils/monomorphize.h"

namespace mxslc::runtime
{
    using runtime_utils::Monomorphizable;

    class Field : public Monomorphizable<Field>, Stringable
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

        explicit Field(TypePtr type) : Field{ModifierList{}, std::move(type), ""} { }

        const ModifierList& modifiers() const { return mods_; }
        bool is_const() const { return mods_.contains(TokenType::Const); }
        bool is_mutable() const { return mods_.contains(TokenType::Mutable); }
        bool is_global() const { return mods_.contains(TokenType::Global); }
        const TypePtr& type() const { return type_; }
        bool has_name() const { return not name_.empty(); }
        const string& name() const { return name_; }

        Field monomorphize(const TypePtr& template_type) const override;

        string to_string() const override;

    private:
        ModifierList mods_;
        TypePtr type_;
        string name_;
    };
}

#endif //MXSLC_FIELD_H
