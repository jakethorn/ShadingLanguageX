//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_TYPEINFO_H
#define MXSLC_TYPEINFO_H

#include <cassert>
#include <utility>

#include "ModifierList.h"
#include "Token.h"
#include "FieldInfo.h"
#include "utils/instantiate_template_types_utils.h"
#include "utils/str_utils.h"

class TypeInfo
{
public:
    inline static string Void = "void"s;
    inline static string Bool = "boolean"s;
    inline static string Int = "integer"s;
    inline static string Float = "float"s;
    inline static string String = "string"s;
    inline static string MultiOutput = "multioutput"s;

    TypeInfo(ModifierList mods, Token name, Token parent, vector<FieldInfo> fields)
        : mods_{std::move(mods)}, name_{std::move(name)}, parent_{std::move(parent)}, fields_{std::move(fields)} { }

    explicit TypeInfo(const string& name) : name_{name} { }
    explicit TypeInfo(Token name) : name_{std::move(name)} { }
    explicit TypeInfo(vector<FieldInfo> fields) : fields_{std::move(fields)} { }

    explicit TypeInfo(const basic_t& val)
    {
        if (std::holds_alternative<bool>(val))
            name_ = Bool;
        if (std::holds_alternative<int>(val))
            name_ = Int;
        if (std::holds_alternative<float>(val))
            name_ = Float;
        if (std::holds_alternative<string>(val))
            name_ = String;
    }

    explicit TypeInfo(const vector<TypeInfoPtr>& fields)
    {
        fields_.reserve(fields.size());
        for (const TypeInfoPtr& field : fields)
            fields_.emplace_back(field);
    }

    [[nodiscard]] const ModifierList& modifiers() const { return mods_; }
    [[nodiscard]] const string& name() const { return name_.lexeme(); }
    [[nodiscard]] const string& parent() const { return parent_.lexeme(); }
    [[nodiscard]] const vector<FieldInfo>& fields() const { return fields_; }
    [[nodiscard]] const Token& name_token() const { return name_; }
    [[nodiscard]] const Token& parent_token() const { return parent_; }

    [[nodiscard]] TypeInfoPtr instantiate_template_types(const TypeInfoPtr& template_type) const
    {
        Token name = name_.instantiate_template_types(template_type);
        vector<FieldInfo> fields = ::instantiate_template_types(fields, template_type);
        return std::make_shared<TypeInfo>(mods_, std::move(name), parent_, std::move(fields));
    }

    [[nodiscard]] size_t field_count() const { return fields_.size(); }
    [[nodiscard]] bool has_fields() const { return field_count() > 0; }
    [[nodiscard]] bool has_field(const string& name) const
    {
        for (const FieldInfo& field : fields_)
        {
            if (field.has_name() and field.name() == name)
                return true;
        }

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
        {
            if (field.has_name() and field.name() == name)
                return field;
        }

        throw CompileError{"Field not found by name"s};
    }
    [[nodiscard]] TypeInfoPtr field_type(const size_t index) const { return field(index).type(); }
    [[nodiscard]] TypeInfoPtr field_type(const string& name) const { return field(name).type(); }
    [[nodiscard]] string field_name(const size_t index) const
    {
        if (field(index).has_name())
            return field(index).name();
        return "field" + str(index);
    }

    void set_initialized() { is_initialized_ = true; }
    [[nodiscard]] bool is_initialized() const { return is_initialized_; }

    [[nodiscard]] bool is_compatible(const TypeInfoPtr& other) const
    {
        assert(is_initialized_);
        assert(other->is_initialized_);
        if (has_fields() and other->has_fields())
        {
            if (field_count() != other->field_count())
                return false;
            for (size_t i = 0; i < field_count(); i++)
            {
                if (not field(i).is_compatible(other->field(i)))
                    return false;
            }
            return true;
        }
        else
        {
            return name_ == other->name_;
        }
    }

    [[nodiscard]] bool is_compatible(const vector<TypeInfoPtr>& other) const
    {
        for (const TypeInfoPtr& type : other)
        {
            if (is_compatible(type))
                return true;
        }

        return false;
    }

    bool operator==(const string& other) const { return name_.lexeme() == other; }
    bool operator!=(const string& other) const { return not (*this == other); }

private:
    ModifierList mods_;
    Token name_;
    Token parent_;
    vector<FieldInfo> fields_;

    bool is_initialized_ = false;
};

inline bool operator==(const TypeInfoPtr& type, const string& other)
{
    if (type)
        return *type == other;
    return false;
}

inline bool operator!=(const TypeInfoPtr& type, const string& other)
{
    return not (type == other);
}

#endif //MXSLC_TYPEINFO_H
