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
    TypeInfo(ModifierList mods, string name, vector<FieldInfo> fields)
        : mods_{std::move(mods)}, name_{std::move(name)}, fields_{std::move(fields)} { }

    explicit TypeInfo(string name) : name_{std::move(name)} { }
    explicit TypeInfo(vector<FieldInfo> fields) : fields_{std::move(fields)} { }
    explicit TypeInfo(const basic_t& val);
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

    template<typename T>
    bool is() const
    {
        if constexpr (std::is_same_v<T, bool>) return name_ == Bool;
        if constexpr (std::is_same_v<T, int>) return name_ == Int;
        if constexpr (std::is_same_v<T, float>) return name_ == Float;
        if constexpr (std::is_same_v<T, string>) return name_ == String;
        if constexpr (std::is_same_v<T, mx::Vector2>) return name_ == Vec2;
        if constexpr (std::is_same_v<T, mx::Vector3>) return name_ == Vec3;
        if constexpr (std::is_same_v<T, mx::Vector4>) return name_ == Vec4;
        if constexpr (std::is_same_v<T, mx::Color3>) return name_ == Color3;
        if constexpr (std::is_same_v<T, mx::Color4>) return name_ == Color4;
        if constexpr (std::is_same_v<T, mx::Matrix33>) return name_ == Mat33;
        if constexpr (std::is_same_v<T, mx::Matrix44>) return name_ == Mat44;
        return false;
    }

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

    static string to_string(const vector<TypeInfoPtr>& types);

    inline static string Bool = "boolean"s;
    inline static string Int = "integer"s;
    inline static string Float = "float"s;
    inline static string String = "string"s;
    inline static string Filename = "filename"s;
    inline static string Vec2 = "vector2"s;
    inline static string Vec3 = "vector3"s;
    inline static string Vec4 = "vector4"s;
    inline static string Color3 = "color3"s;
    inline static string Color4 = "color4"s;
    inline static string Mat33 = "matrix33"s;
    inline static string Mat44 = "matrix44"s;
    inline static string MultiOutput = "multioutput"s;
    inline static string Void = "void"s;

protected:
    void set_resolved() { is_resolved_ = true; }
    
private:
    ModifierList mods_;
    string name_;
    vector<FieldInfo> fields_;

    bool is_resolved_ = false;
};

class ResolvedTypeInfo : public TypeInfo
{
public:
    explicit ResolvedTypeInfo(string name) : TypeInfo{std::move(name)}
    {
        set_resolved();
    }
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
