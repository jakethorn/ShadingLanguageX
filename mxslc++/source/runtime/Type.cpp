//
// Created by jaket on 18/01/2026.
//

#include <cassert>

#include "Type.h"

#include "CompileError.h"
#include "utils/instantiate_template_types_utils.h"
#include "utils/str_utils.h"

#define type_def(t) TypePtr Type::t = resolve(TypeName::t);
type_def(Bool)
type_def(Int)
type_def(Float)
type_def(String)
type_def(Filename)
type_def(Vec2)
type_def(Vec3)
type_def(Vec4)
type_def(Color3)
type_def(Color4)
type_def(Mat3)
type_def(Mat4)
type_def(Void)
type_def(Auto)
#undef type_def

Type::Type(string name) : name_{std::move(name)} { }

Type::Type(vector<Field> fields) : fields_{std::move(fields)}
{
    validate();
}

Type::Type(const vector<TypePtr>& fields)
{
    fields_.reserve(fields.size());
    for (const TypePtr& field : fields)
        fields_.emplace_back(field);
}

Type::Type(string name, vector<Field> fields)
    : name_{std::move(name)}, fields_{std::move(fields)}
{
    validate();
}

Type::Type(string name, vector<Field> fields, vector<weak_ptr<Function>> methods)
    : name_{std::move(name)}, fields_{std::move(fields)}, methods_{std::move(methods)}
{
    validate();
}

Type::Type(const TypePtr& field_type, const size_t field_count)
{
    fields_.reserve(field_count);
    for (size_t i = 0; i < field_count; ++i)
        fields_.emplace_back(field_type);
}

TypePtr Type::instantiate_template_types(const TypePtr& template_type) const
{
    string name = ::instantiate_template_types(name_, template_type);
    vector<Field> fields = ::instantiate_template_types(fields_, template_type);
    return std::make_shared<Type>(std::move(name), std::move(fields));
}

void Type::add_field(Field field)
{
    fields_.push_back(std::move(field));
    validate();
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

    throw CompileError{"Expression of type " + str() + " does not have a field with the name " + name};
}

void Type::add_method(weak_ptr<Function> method)
{
    methods_.push_back(std::move(method));
}

vector<FuncPtr> Type::methods() const
{
    return lock(methods_);
}

size_t Type::component_count() const
{
    assert(is_vector());
    if (is<mx::Vector2>()) return 2;
    if (is<mx::Vector3>()) return 3;
    if (is<mx::Vector4>()) return 4;
    if (is<mx::Color3>()) return 3;
    if (is<mx::Color4>()) return 4;
    return 0;
}

namespace
{
    bool is_vector_compatible(const Type& vec_type, const Type& type)
    {
        assert(vec_type.is_vector());

        if (type.has_name())
            return false;

        if (not type.has_fields())
            return false;

        for (size_t i = 0; i < type.field_count(); i++)
        {
            if (not type.field_type(i)->is<float>())
                return false;
        }

        return vec_type.component_count() == type.field_count();
    }
}

bool Type::is_compatible(const TypePtr& other) const
{
    if (other == nullptr)
        return false;

    assert(is_resolved_);
    assert(other->is_resolved_);

    if (is_auto() or other->is_auto())
        return true;

    if (is_vector() and is_vector_compatible(*this, *other))
        return true;

    if (other->is_vector() and is_vector_compatible(*other, *this))
        return true;

    if (has_name() and other->has_name())
        return name_ == other->name_;

    if (field_count() != other->field_count())
        return false;

    for (size_t i = 0; i < field_count(); i++)
    {
        if (not field_type(i)->is_compatible(other->field_type(i)))
            return false;
    }

    return true;
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

bool Type::is_equal(const TypePtr& other, const bool field_names) const
{
    if (other == nullptr)
        return false;

    assert(is_resolved_);
    assert(other->is_resolved_);

    if (name_ != other->name_)
        return false;

    if (field_count() != other->field_count())
        return false;

    for (size_t i = 0; i < field_count(); i++)
    {
        if (not field_type(i)->is_equal(other->field_type(i), field_names))
            return false;
        if (field_names and field_name(i) != other->field_name(i))
            return false;
    }

    return true;
}

bool Type::is_in(const vector<TypePtr>& types) const
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
    vector<TypePtr> compatibles;
    for (const TypePtr& type : types)
    {
        if (is_equal(type))
            return type;

        if (is_compatible(type))
            compatibles.push_back(type);
    }

    return compatibles.size() == 1 ? compatibles[0] : nullptr;
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

TypePtr Type::of(const mx::NodeGraphPtr& node_graph)
{
    if (node_graph->getOutputCount() == 1)
        return Type::of(node_graph->getOutputs()[0]);

    vector<TypePtr> types;
    for (const mx::OutputPtr& output : node_graph->getOutputs())
        types.push_back(Type::of(output));
    TypePtr result = std::make_shared<Type>(types);
    result->set_resolved();
    return result;
}

TypePtr Type::of(const mx::TypedElementPtr& value)
{
    return resolve(value->getType());
}

TypePtr Type::of(const mxslc::Variable& var)
{
    if (var.has_type())
        return std::make_shared<Type>(var.type());
    vector<Field> fields;
    fields.reserve(var.children().size());
    for (const mxslc::VariablePtr& child : var.children())
        fields.emplace_back(of(*child), child->name());
    TypePtr type = std::make_shared<Type>(std::move(fields));
    return type;
}

TypePtr Type::unnamed_struct(TypePtr field_type, const size_t field_count)
{
    assert(field_type->is_resolved());
    assert(field_count > 0);

    vector<Field> fields;
    fields.reserve(field_count);
    for (size_t i = 0; i < field_count; ++i)
        fields.emplace_back(field_type);
    TypePtr type = std::make_shared<Type>(std::move(fields));
    type->set_resolved();
    return type;
}

TypePtr Type::resolve(const string& name)
{
    TypePtr type = std::make_shared<Type>(name);
    type->set_resolved();
    return type;
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

void Type::validate() const
{
    if (not has_fields())
        return;

    const bool has_names = fields_[0].has_name();
    for (const Field& field : fields_)
    {
        if (field.has_name() != has_names)
            throw CompileError{"Either all type fields must be named or none them\nType: " + str()};
    }
}
