//
// Created by jaket on 16/01/2026.
//

#ifndef MXSLC_TYPE_H
#define MXSLC_TYPE_H

#include <MaterialXCore/Node.h>

#include "runtime/Field.h"
#include "runtime/utils/TypeName.h"

namespace mxslc::runtime
{
    using runtime_utils::TypeName;
    using runtime_utils::Monomorphizable;

    class Type : public Monomorphizable<TypePtr>, public Stringable
    {
        friend class Scope;

    public:
        explicit Type(string name);
        explicit Type(vector<Field> fields);
        explicit Type(const vector<TypePtr>& fields);
        Type(string name, vector<Field> fields);
        Type(string name, vector<Field> fields, vector<weak_ptr<Function>> methods);
        Type(const TypePtr& field_type, size_t field_count);

        bool has_name() const { return not name_.empty(); }
        const string& name() const { return name_; }

        TypePtr monomorphize(const TypePtr& template_type) const override;

        void add_field(Field field);
        size_t field_count() const { return fields_.size(); }
        bool has_fields() const { return field_count() > 0; }
        bool has_field(const string& name) const;
        const vector<Field>& fields() const { return fields_; }
        const Field& field(size_t index) const;
        const Field& field(const string& name) const;
        const string& field_name(const size_t index) const { return field(index).name(); }
        size_t field_index(const string& name) const;
        TypePtr field_type(const size_t index) const { return field(index).type(); }
        TypePtr field_type(const string& name) const { return field(name).type(); }

        void add_method(weak_ptr<Function> method);
        vector<FuncPtr> methods() const;

        size_t component_count() const;

        template<typename T>
        bool is() const { return name_ == TypeName::of<T>(); }
        bool is_void() const { return name_ == TypeName::Void; }
        bool is_auto() const { return name_ == TypeName::Auto; }
        bool is_primitive() const { return has_name() and not has_fields(); }
        bool is_single() const { return TypeName::is_single(name_); }
        bool is_vector() const { return TypeName::is_vector(name_); }
        bool is_matrix() const { return TypeName::is_matrix(name_); }

        bool is_resolved() const { return is_resolved_; }

        bool is_compatible(const TypePtr& other) const;
        bool is_compatible(const vector<TypePtr>& types) const;
        bool equals(const TypePtr& other, bool field_names = false) const;
        bool is_in(const vector<TypePtr>& types) const;

        TypePtr find_unique_compatible(const vector<TypePtr>& types) const;

        string to_string() const override;
        string full_str() const;

        bool operator==(const string& other) const { return name_ == other; }
        bool operator!=(const string& other) const { return not (*this == other); }

        static TypePtr Bool;
        static TypePtr Int;
        static TypePtr Float;
        static TypePtr String;
        static TypePtr Filename;
        static TypePtr Vec2;
        static TypePtr Vec3;
        static TypePtr Vec4;
        static TypePtr Color3;
        static TypePtr Color4;
        static TypePtr Mat3;
        static TypePtr Mat4;
        static TypePtr Void;
        static TypePtr Auto;

        template<typename T>
        static TypePtr of() { return resolve(TypeName::of<T>()); }
        static TypePtr of(const mx::NodeGraphPtr& node_graph);
        static TypePtr of(const mx::TypedElementPtr& value);
        static TypePtr unnamed_struct(TypePtr field_type, size_t field_count);

    private:
        string name_;
        vector<Field> fields_;
        vector<weak_ptr<Function>> methods_;

        bool is_resolved_{false};

        void set_resolved() { is_resolved_ = true; }
        static TypePtr resolve(const string& name);

        void validate() const;
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
}

#endif //MXSLC_TYPE_H
