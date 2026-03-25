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
    FieldInfo(ModifierList mods, TypeInfoPtr type, optional<Token> name, ExprPtr initializer)
        : mods_{std::move(mods)}, type_{std::move(type)}, name_{std::move(name)}, initializer_{std::move(initializer)} { }

    explicit FieldInfo(TypeInfoPtr type)
        : type_{std::move(type)} { }

    [[nodiscard]] const ModifierList& modifiers() const { return mods_; }
    [[nodiscard]] TypeInfoPtr type() const { return type_; }
    [[nodiscard]] bool has_name() const { return name_.has_value(); }
    [[nodiscard]] const string& name() const { return name_->lexeme(); }
    [[nodiscard]] ExprPtr initializer() const { return initializer_; }
    [[nodiscard]] const Token& name_token() const { return *name_;}

    [[nodiscard]] FieldInfo instantiate_template_types(const TypeInfoPtr& template_type) const;

    [[nodiscard]] bool is_compatible(const FieldInfo& other) const;

private:
    ModifierList mods_;
    TypeInfoPtr type_;
    optional<Token> name_;
    ExprPtr initializer_;
};

#endif //MXSLC_FIELDINFO_H
