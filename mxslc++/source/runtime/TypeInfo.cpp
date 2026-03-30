//
// Created by jaket on 18/01/2026.
//

#include <cassert>

#include "TypeInfo.h"
#include "utils/instantiate_template_types_utils.h"
#include "utils/str_utils.h"

TypeInfoPtr TypeInfo::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    Token name = name_.instantiate_template_types(template_type);
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

    throw CompileError{name_, "Expression of type " + str() + " does not have a field at index " + ::str(index)};
}

const FieldInfo& TypeInfo::field(const Token& name) const
{
    for (const FieldInfo& field : fields_)
    {
        if (field.has_name() and field.name() == name.lexeme())
            return field;
    }

    throw CompileError{name, "Expression of type " + str() + " does not have a field with the name " + name.lexeme()};
}

size_t TypeInfo::field_index(const Token& name) const
{
    size_t i = 0;
    for (const FieldInfo& field : fields_)
    {
        if (field.has_name() and field.name() == name.lexeme())
            return i;
        ++i;
    }

    throw CompileError{name, "Expression of type " + str() + " does not have a field with the name " + name.lexeme()};
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

TypeInfoPtr TypeInfo::find_unique_compatible(const vector<TypeInfoPtr> &types) const
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
        return name();

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
