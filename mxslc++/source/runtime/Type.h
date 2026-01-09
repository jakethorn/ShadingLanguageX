//
// Created by jaket on 13/11/2025.
//

#ifndef FENNEC_TYPE_H
#define FENNEC_TYPE_H

#include "utils/common.h"
#include <utility>

#include "utils/template_utils.h"

class Token;

class Type
{
    using TypePtr = unique_ptr<Type>;

public:
    Type();
    Type(string name);
    Type(const Token& token);
    explicit Type(const basic_t& value);
    explicit Type(vector<TypePtr> subtypes);
    explicit Type(const vector<string>& subtypes);

    Type(const Type& other);
    Type& operator=(const Type& other);

    [[nodiscard]] Type instantiate_template_type(const Type& template_type) const;

    [[nodiscard]] bool is_complex() const { return not subtypes_.empty(); }
    [[nodiscard]] const string& str() const { return name_; }

    bool operator==(const Type& other) const { return str() == other.str(); }
    bool operator==(const string& other) const { return str() == other; }

    bool operator!=(const Type& other) const { return not (*this == other); }
    bool operator!=(const string& other) const { return not (*this == other); }

    static optional<Type> instantiate_template_type(const optional<Type>& type, const Type& template_type)
    {
        if (type)
            return type->instantiate_template_type(template_type);
        return std::nullopt;
    }

private:
    [[nodiscard]] string complex_name() const;

    string name_;
    vector<TypePtr> subtypes_;
};

template<>
struct std::hash<Type>
{
    size_t operator()(const Type& type) const noexcept
    {
        return std::hash<std::string>{}(type.str());
    }
};

#endif //FENNEC_TYPE_H
