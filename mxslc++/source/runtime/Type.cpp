//
// Created by jaket on 18/01/2026.
//

#include <cassert>

#include "runtime/Type.h"

#include "runtime/utils/monomorphize.h"
#include "utils/string_utils.h"
#include "errors/CompileError.h"
#include "runtime/interface.h"

namespace mxslc::runtime
{
    using container_utils::lock;

    TypePtr Type::Bool = resolve("boolean");
    TypePtr Type::Int = resolve("integer");
    TypePtr Type::Float = resolve("float");
    TypePtr Type::String = resolve("string");
    TypePtr Type::Filename = resolve("filename");
    TypePtr Type::Vec2 = resolve("vector2");
    TypePtr Type::Vec3 = resolve("vector3");
    TypePtr Type::Vec4 = resolve("vector4");
    TypePtr Type::Color3 = resolve("color3");
    TypePtr Type::Color4 = resolve("color4");
    TypePtr Type::Mat3 = resolve("matrix33");
    TypePtr Type::Mat4 = resolve("matrix44");
    TypePtr Type::Void = resolve("void");
    TypePtr Type::Auto = resolve("auto");

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

    TypePtr Type::monomorphize(const TypePtr& template_type) const
    {
        auto&& [name, fields] = runtime_utils::monomorphize_all(template_type, name_, fields_);
        return create_type(std::move(name), std::move(fields));
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

        throw CompileError{"Expression of type " + to_string() + " does not have a field at index " + std::to_string(index)};
    }

    const Field& Type::field(const string& name) const
    {
        for (const Field& field : fields_)
        {
            if (field.has_name() and field.name() == name)
                return field;
        }

        throw CompileError{"Expression of type " + to_string() + " does not have a field with the name " + name};
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

        throw CompileError{"Expression of type " + to_string() + " does not have a field with the name " + name};
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
        if (is<mx::Vector2>()) return 2;
        if (is<mx::Vector3>()) return 3;
        if (is<mx::Vector4>()) return 4;
        if (is<mx::Color3>()) return 3;
        if (is<mx::Color4>()) return 4;
        if (is<mx::Matrix33>()) return 9;
        if (is<mx::Matrix44>()) return 16;
        return 1;
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

    bool Type::equals(const TypePtr& other, const bool field_names) const
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
            if (not field_type(i)->equals(other->field_type(i), field_names))
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
            if (equals(type))
                return true;
        }

        return false;
    }

    TypePtr Type::find_unique_compatible(const vector<TypePtr>& types) const
    {
        vector<TypePtr> compatibles;
        for (const TypePtr& type : types)
        {
            if (equals(type))
                return type;

            if (is_compatible(type))
                compatibles.push_back(type);
        }

        return compatibles.size() == 1 ? compatibles[0] : nullptr;
    }

    TypePtr Type::of(const mx::NodeGraphPtr& node_graph)
    {
        if (node_graph->getOutputCount() == 1)
            return Type::of(node_graph->getOutputs()[0]);

        vector<TypePtr> types;
        for (const mx::OutputPtr& output : node_graph->getOutputs())
            types.push_back(Type::of(output));
        TypePtr result = create_type(types);
        result->set_resolved();
        return result;
    }

    TypePtr Type::of(const mx::TypedElementPtr& value)
    {
        return resolve(value->getType());
    }

    TypePtr Type::unnamed_struct(TypePtr field_type, const size_t field_count)
    {
        assert(field_type->is_resolved());
        assert(field_count > 0);

        vector<Field> fields;
        fields.reserve(field_count);
        for (size_t i = 0; i < field_count; ++i)
            fields.emplace_back(field_type);
        TypePtr type = create_type(std::move(fields));
        type->set_resolved();
        return type;
    }

    TypePtr Type::resolve(const string& name)
    {
        TypePtr type = create_type(name);
        type->set_resolved();
        return type;
    }

    void Type::validate() const
    {
        if (not has_fields())
            return;

        const bool has_names = fields_[0].has_name();
        for (const Field& field : fields_)
        {
            if (field.has_name() != has_names)
                throw CompileError{"Either all type fields must be named or none them\nType: " + to_string()};
        }
    }

    string Type::to_string() const
    {
        if (has_name())
            return name_;

        return "{" + join(fields_, ", ") + "}";
    }

    string Type::full_str() const
    {
        string result;
        if (has_name())
            result += name_;

        if (not has_fields())
            return result;

        result += "{";
        for (const Field& field : fields_)
        {
            result += field.to_string();
            result += ", ";
        }

        result.pop_back();
        result.pop_back();

        result += "}";
        return result;
    }
}
