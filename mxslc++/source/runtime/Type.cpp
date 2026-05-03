//
// Created by jaket on 18/01/2026.
//

#include <cassert>

#include "Type.h"

#include "CompileError.h"
#include "utils/instantiate_template_types_utils.h"
#include "utils/str_utils.h"

Type::Type(const primitive_t& val)
{
#define START_INIT if constexpr (false) { }
#define INIT_PRIM(t, n) else if (std::holds_alternative<t>(val)) name_ = n;

    START_INIT
    INIT_PRIM(bool, Bool)
    INIT_PRIM(int, Int)
    INIT_PRIM(float, Float)
    INIT_PRIM(string, String)
    INIT_PRIM(mx::Vector2, Vec2)
    INIT_PRIM(mx::Vector3, Vec3)
    INIT_PRIM(mx::Vector4, Vec4)
    INIT_PRIM(mx::Color3, Color3)
    INIT_PRIM(mx::Color4, Color4)
    INIT_PRIM(mx::Matrix33, Mat33)
    INIT_PRIM(mx::Matrix44, Mat44)

#undef INIT_PRIM
#undef START_INIT

    set_resolved();
}

TypePtr Type::instantiate_template_types(const TypePtr& template_type) const
{
    string name = ::instantiate_template_types(name_, template_type);
    vector<Field> fields = ::instantiate_template_types(fields, template_type);
    return std::make_shared<Type>(mods_, std::move(name), std::move(fields));
}

bool Type::has_field(const string& name) const
{
    for (const Field& field : fields_)
    {
        if (field.has_name() and field.name() == name)
            return true;
    }

    return false;
}

const Field& Type::field(const size_t index) const
{
    if (index < fields_.size())
        return fields_.at(index);

    throw CompileError{"Expression of type " + str() + " does not have a field at index " + ::str(index)};
}

const Field& Type::field(const string& name) const
{
    for (const Field& field : fields_)
    {
        if (field.has_name() and field.name() == name)
            return field;
    }

    throw CompileError{"Expression of type " + str() + " does not have a field with the name " + name};
}

size_t Type::field_index(const string& name) const
{
    size_t i = 0;
    for (const Field& field : fields_)
    {
        if (field.has_name() and field.name() == name)
            return i;
        ++i;
    }

    throw CompileError{"Expression of type " + str() + " does not have a field with the name "};
}

bool Type::is_compatible(const TypePtr& other) const
{
    assert(is_resolved_);
    assert(other->is_resolved_);

    if (has_name() and other->has_name())
    {
        return name_ == other->name_;
    }

    if (has_fields() and other->has_fields())
    {
        if (field_count() != other->field_count())
            return false;
        for (size_t i = 0; i < field_count(); i++)
        {
            if (not field_type(i)->is_compatible(other->field_type(i)))
                return false;
        }
        return true;
    }

    return false;
}

bool Type::is_compatible(const vector<TypePtr>& types) const
{
    for (const TypePtr& type : types)
    {
        if (is_compatible(type))
            return true;
    }

    return false;
}

bool Type::is_equal(const TypePtr &other) const
{
    assert(is_resolved_);
    assert(other->is_resolved_);

    if (name_ != other->name_)
    {
        return false;
    }

    if (field_count() != other->field_count())
        return false;

    for (size_t i = 0; i < field_count(); i++)
    {
        if (not field_type(i)->is_equal(other->field_type(i)))
            return false;
    }

    return true;
}

bool Type::is_in(const vector<TypePtr> &types) const
{
    for (const TypePtr& type : types)
    {
        if (is_equal(type))
            return true;
    }

    return false;
}

TypePtr Type::find_unique_compatible(const vector<TypePtr>& types) const
{
    TypePtr compatible = nullptr;
    for (const TypePtr& type : types)
    {
        if (is_compatible(type))
        {
            if (compatible == nullptr)
                compatible = type;
            else
                return nullptr;
        }
    }

    return compatible;
}

string Type::str() const
{
    if (has_name())
        return name_;

    string result = "{";
    for (const Field& field : fields_)
    {
        result += field.str();
        result += ", ";
    }

    result.pop_back();
    result.pop_back();

    result += "}";
    return result;
}

string Type::to_string(const vector<TypePtr>& types)
{
    if (types.empty())
        return ""s;

    if (types.size() == 1)
        return types[0]->str();

    string result = "(";
    for (size_t i = 0; i < types.size(); ++i)
    {
        result += types[i]->str();
        if (i < types.size() - 1)
            result += ", ";
    }
    result += ")";
    return result;
}
