//
// Created by jaket on 13/11/2025.
//

#ifndef FENNEC_TYPE_H
#define FENNEC_TYPE_H

#include "utils/common.h"
#include <utility>

class Token;

class Type
{
public:
    Type() : Type{"__not_a_type__"s} { }
    Type(string name) : name_{std::move(name)} { }
    Type(const Token& token);
    explicit Type(const basic_t& value);

    [[nodiscard]] Type instantiate_template_type(const Type& template_type) const
    {
        return name_ == "T" ? template_type : *this;
    }

    [[nodiscard]] const string& name() const { return name_; }

    bool operator==(const Type& other) const { return name_ == other.name_; }
    bool operator==(const string& other) const { return name_ == other; }

    bool operator!=(const Type& other) const { return not (*this == other); }
    bool operator!=(const string& other) const { return not (*this == other); }

    static optional<Type> instantiate_template_type(const optional<Type>& type, const Type& template_type)
    {
        if (type)
            return type->instantiate_template_type(template_type);
        return std::nullopt;
    }

private:
    string name_;
};

template<>
struct std::hash<Type>
{
    size_t operator()(const Type& type) const noexcept
    {
        return std::hash<std::string>{}(type.name());
    }
};

#endif //FENNEC_TYPE_H
