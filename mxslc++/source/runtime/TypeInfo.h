//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_TYPEINFO_H
#define MXSLC_TYPEINFO_H

#include "ModifierList.h"
#include "Token.h"
#include "FieldInfo.h"
#include "utils/instantiate_template_types_utils.h"

class TypeInfo
{
public:
    struct Basic
    {
        inline static string Bool = "boolean"s;
        inline static string Int = "integer"s;
        inline static string Float = "float"s;
        inline static string String = "string"s;
    };

    TypeInfo(ModifierList mods, Token name, Token parent, vector<FieldInfo> fields)
        : mods_{std::move(mods)}, name_{std::move(name)}, parent_{std::move(parent)}, fields_{std::move(fields)} { }

    TypeInfo(const string& name)
        : name_{name} { }

    [[nodiscard]] const ModifierList& modifiers() const { return mods_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] const string& parent() const { return parent_.lexeme(); }
    [[nodiscard]] const vector<FieldInfo>& fields() const { return fields_; }
    [[nodiscard]] const Token& name_token() const { return name_; }

    [[nodiscard]] TypeInfoPtr instantiate_template_types(const TypeInfoPtr& template_type) const
    {
        Token name = name_.instantiate_template_types(template_type);
        vector<FieldInfo> fields = ::instantiate_template_types(fields, template_type);
        return std::make_shared<TypeInfo>(mods_, std::move(name), parent_, std::move(fields));
    }

    [[nodiscard]] size_t field_count() const { return fields_.size(); }
    [[nodiscard]] bool has_field(const string& name) const
    {
        for (const FieldInfo& field : fields_)
            if (field.name() == name)
                return true;
        return false;
    }
    [[nodiscard]] const FieldInfo& field(const size_t index) const
    {
        if (index < fields_.size())
            return fields_.at(index);
        throw CompileError{"Field not found by index"s};
    }
    [[nodiscard]] const FieldInfo& field(const string& name) const
    {
        for (const FieldInfo& field : fields_)
            if (field.name() == name)
                return field;
        throw CompileError{"Field not found by name"s};
    }
    [[nodiscard]] TypeInfoPtr field_type(const size_t index) const { return field(index).type(); }

    static TypeInfoPtr create_unnamed_struct_type(const vector<TypeInfoPtr>& types)
    {
        vector<FieldInfo> fields;
        fields.reserve(types.size());
        for (const TypeInfoPtr& type : types)
            fields.emplace_back(ModifierList{}, type, Token{}, nullptr);
        return std::make_shared<TypeInfo>(ModifierList{}, Token{}, Token{}, std::move(fields));
    }

private:
    ModifierList mods_;
    Token name_;
    Token parent_;
    vector<FieldInfo> fields_;
};

#endif //MXSLC_TYPEINFO_H
