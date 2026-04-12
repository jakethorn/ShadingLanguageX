//
// Created by jaket on 18/01/2026.
//

#include <cassert>

#include "TypeInfo.h"

#include "CompileError.h"
#include "utils/instantiate_template_types_utils.h"
#include "utils/str_utils.h"

TypeInfo::TypeInfo(const basic_t& val)
{
#define START_INIT if constexpr (false) { }
#define INIT_BASIC(t, n) else if (std::holds_alternative<t>(val)) name_ = n;

    START_INIT
    INIT_BASIC(bool, Bool)
    INIT_BASIC(int, Int)
    INIT_BASIC(float, Float)
    INIT_BASIC(string, String)
    INIT_BASIC(mx::Vector2, Vec2)
    INIT_BASIC(mx::Vector3, Vec3)
    INIT_BASIC(mx::Vector4, Vec4)
    INIT_BASIC(mx::Color3, Color3)
    INIT_BASIC(mx::Color4, Color4)
    INIT_BASIC(mx::Matrix33, Mat33)
    INIT_BASIC(mx::Matrix44, Mat44)

#undef INIT_BASIC
#undef START_INIT

    set_resolved();
}

TypeInfoPtr TypeInfo::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    string name = ::instantiate_template_types(name_, template_type);
    vector<FieldInfo> fields = ::instantiate_template_types(fields, template_type);
    return std::make_shared<TypeInfo>(mods_, std::move(name), std::move(fields));
}

bool TypeInfo::has_field(const string& name) const
{
    for (const FieldInfo& field : fields_)
    {
        if (field.has_name() and field.name() == name)
            return true;
    }

    return false;
}

const FieldInfo& TypeInfo::field(const size_t index) const
{
    if (index < fields_.size())
        return fields_.at(index);

    throw CompileError{"Expression of type " + str() + " does not have a field at index " + ::str(index)};
}

const FieldInfo& TypeInfo::field(const string& name) const
{
    for (const FieldInfo& field : fields_)
    {
        if (field.has_name() and field.name() == name)
            return field;
    }

    throw CompileError{"Expression of type " + str() + " does not have a field with the name " + name};
}

size_t TypeInfo::field_index(const string& name) const
{
    size_t i = 0;
    for (const FieldInfo& field : fields_)
    {
        if (field.has_name() and field.name() == name)
            return i;
        ++i;
    }

    throw CompileError{"Expression of type " + str() + " does not have a field with the name "};
}

bool TypeInfo::is_compatible(const TypeInfoPtr& other) const
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

bool TypeInfo::is_compatible(const vector<TypeInfoPtr>& types) const
{
    for (const TypeInfoPtr& type : types)
    {
        if (is_compatible(type))
            return true;
    }

    return false;
}

bool TypeInfo::is_equal(const TypeInfoPtr &other) const
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

bool TypeInfo::is_in(const vector<TypeInfoPtr> &types) const
{
    for (const TypeInfoPtr& type : types)
    {
        if (is_equal(type))
            return true;
    }

    return false;
}

TypeInfoPtr TypeInfo::find_unique_compatible(const vector<TypeInfoPtr>& types) const
{
    TypeInfoPtr compatible = nullptr;
    for (const TypeInfoPtr& type : types)
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

string TypeInfo::str() const
{
    if (has_name())
        return name_;

    string result = "{";
    for (const FieldInfo& field : fields_)
    {
        result += field.str();
        result += ", ";
    }

    result.pop_back();
    result.pop_back();

    result += "}";
    return result;
}

TypeInfoPtr TypeInfo::resolved_void()
{
    TypeInfoPtr type = std::make_shared<TypeInfo>(Void);
    type->set_resolved();
    return type;
}

string TypeInfo::to_string(const vector<TypeInfoPtr>& types)
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
