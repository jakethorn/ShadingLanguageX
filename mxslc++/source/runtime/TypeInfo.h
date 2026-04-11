//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_TYPEINFO_H
#define MXSLC_TYPEINFO_H

#include <utility>

#include "ModifierList.h"
#include "FieldInfo.h"

class TypeInfo
{
    friend class Scope;

public:
    inline static string Void = "void"s;
    inline static string Bool = "boolean"s;
    inline static string Int = "integer"s;
    inline static string Float = "float"s;
    inline static string String = "string"s;
    inline static string Filename = "filename"s;
    inline static string MultiOutput = "multioutput"s;

    TypeInfo(ModifierList mods, string name, vector<FieldInfo> fields)
        : mods_{std::move(mods)}, name_{std::move(name)}, fields_{std::move(fields)} { }

    explicit TypeInfo(string name) : name_{std::move(name)} { }
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

        set_resolved();
    }

    explicit TypeInfo(const vector<TypeInfoPtr>& fields)
    {
        fields_.reserve(fields.size());
        for (const TypeInfoPtr& field : fields)
            fields_.emplace_back(field);
    }

    const ModifierList& modifiers() const { return mods_; }
    bool has_name() const { return not name_.empty(); }
    const string& name() const { return name_; }
    const vector<FieldInfo>& fields() const { return fields_; }

    TypeInfoPtr instantiate_template_types(const TypeInfoPtr& template_type) const;

    size_t field_count() const { return fields_.size(); }
    bool has_fields() const { return field_count() > 0; }
    bool has_field(const string& name) const;

    const FieldInfo& field(size_t index) const;
    const FieldInfo& field(const string& name) const;
    const string& field_name(const size_t index) const { return field(index).name(); }
    size_t field_index(const string& name) const;
    TypeInfoPtr field_type(const size_t index) const { return field(index).type(); }
    TypeInfoPtr field_type(const string& name) const { return field(name).type(); }

    bool is_basic() const { return has_name() and not has_fields(); }
    bool is_resolved() const { return is_resolved_; }
    bool is_compatible(const TypeInfoPtr& other) const;
    bool is_compatible(const vector<TypeInfoPtr>& types) const;
    bool is_equal(const TypeInfoPtr& other) const;
    bool is_in(const vector<TypeInfoPtr>& types) const;

    TypeInfoPtr find_unique_compatible(const vector<TypeInfoPtr>& types) const;

    string str() const;

    bool operator==(const string& other) const { return name_ == other; }
    bool operator!=(const string& other) const { return not (*this == other); }

    static TypeInfoPtr resolved_void();
    static string to_string(const vector<TypeInfoPtr>& types);

private:
    void set_resolved() { is_resolved_ = true; }

    ModifierList mods_;
    string name_;
    vector<FieldInfo> fields_;

    bool is_resolved_ = false;
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
