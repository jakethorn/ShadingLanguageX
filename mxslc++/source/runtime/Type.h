//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_TYPEINFO_H
#define MXSLC_TYPEINFO_H

#include <utility>

#include "Field.h"

class Type
{
    friend class Scope;

public:
    Type(string name, vector<Field> fields, vector<weak_ptr<Function>> methods)
        : name_{std::move(name)}, fields_{std::move(fields)}, methods_{std::move(methods)} { }

    Type(string name, vector<Field> fields)
        : name_{std::move(name)}, fields_{std::move(fields)} { }

    explicit Type(string name) : name_{std::move(name)} { }
    explicit Type(vector<Field> fields) : fields_{std::move(fields)} { }
    explicit Type(const primitive_t& val);
    explicit Type(const vector<TypePtr>& fields)
    {
        fields_.reserve(fields.size());
        for (const TypePtr& field : fields)
            fields_.emplace_back(field);
    }

    bool has_name() const { return not name_.empty(); }
    const string& name() const { return name_; }
    const vector<Field>& fields() const { return fields_; }

    TypePtr instantiate_template_types(const TypePtr& template_type) const;

    void add_field(Field field) { fields_.push_back(std::move(field)); }
    size_t field_count() const { return fields_.size(); }
    bool has_fields() const { return field_count() > 0; }
    bool has_field(const string& name) const;
    const Field& field(size_t index) const;
    const Field& field(const string& name) const;
    const string& field_name(const size_t index) const { return field(index).name(); }
    size_t field_index(const string& name) const;
    TypePtr field_type(const size_t index) const { return field(index).type(); }
    TypePtr field_type(const string& name) const { return field(name).type(); }

    void add_method(weak_ptr<Function> method)
    {
        methods_.push_back(std::move(method));
    }

    const vector<weak_ptr<Function>>& methods() const
    {
        return methods_;
    }

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

    bool is_primitive() const { return has_name() and not has_fields(); }
    bool is_resolved() const { return is_resolved_; }
    bool is_compatible(const TypePtr& other) const;
    bool is_compatible(const vector<TypePtr>& types) const;
    bool is_equal(const TypePtr& other) const;
    bool is_in(const vector<TypePtr>& types) const;

    TypePtr find_unique_compatible(const vector<TypePtr>& types) const;

    string str() const;

    bool operator==(const string& other) const { return name_ == other; }
    bool operator!=(const string& other) const { return not (*this == other); }

    static string to_string(const vector<TypePtr>& types);

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
    string name_;
    vector<Field> fields_;
    vector<weak_ptr<Function>> methods_;

    bool is_resolved_ = false;
};

class ResolvedTypeInfo : public Type
{
public:
    explicit ResolvedTypeInfo(string name) : Type{std::move(name)}
    {
        set_resolved();
    }
};

inline bool operator==(const TypePtr& type, const string& other)
{
    if (type)
        return *type == other;
    return false;
}

inline bool operator!=(const TypePtr& type, const string& other)
{
    return not (type == other);
}

#endif //MXSLC_TYPEINFO_H
