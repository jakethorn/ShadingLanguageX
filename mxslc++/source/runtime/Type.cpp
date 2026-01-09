//
// Created by jaket on 07/12/2025.
//

#include "Type.h"

#include <cassert>

#include "CompileError.h"
#include "Token.h"

Type::Type() : Type{"__not_a_type__"s} { }
Type::Type(string name) : name_{std::move(name)} { }
Type::Type(const Token& token) : Type{token.lexeme()} { }

namespace
{
    string to_type(const basic_t& value)
    {
        if (std::holds_alternative<bool>(value))
            return "bool"s;
        if (std::holds_alternative<int>(value))
            return "int"s;
        if (std::holds_alternative<float>(value))
            return "float"s;
        if (std::holds_alternative<string>(value))
            return "string"s;
        throw CompileError{"Invalid literal type"s};
    }
}

Type::Type(const basic_t& value) : Type{to_type(value)}
{

}

Type::Type(vector<TypePtr> subtypes)
{
    assert(not subtypes.empty());

    if (subtypes.size() == 1)
    {
        subtypes_ = std::move(subtypes[0]->subtypes_);
        name_ = subtypes[0]->name_;
    }
    else
    {
        subtypes_ = std::move(subtypes);
        name_ = complex_name();
    }
}

namespace
{
    using TypePtr = unique_ptr<Type>;
    vector<TypePtr> to_types(const vector<string>& subtypes)
    {
        vector<TypePtr> types;
        types.reserve(subtypes.size());
        for (const string& subtype : subtypes)
            types.push_back(std::make_unique<Type>(subtype));
        return types;
    }
}

Type::Type(const vector<string>& subtypes) : Type{to_types(subtypes)}
{

}

Type::Type(const Type& other) : name_{other.name_}
{
    subtypes_.reserve(other.subtypes_.size());
    for (const TypePtr& subtype : other.subtypes_)
        subtypes_.push_back(std::make_unique<Type>(*subtype));
}

Type& Type::operator=(const Type& other)
{
    if (this == &other)
        return *this;
    name_ = other.name_;
    subtypes_.clear();
    subtypes_.reserve(other.subtypes_.size());
    for (const TypePtr& subtype : other.subtypes_)
        subtypes_.push_back(std::make_unique<Type>(*subtype));
    return *this;
}


[[nodiscard]] Type Type::instantiate_template_type(const Type& template_type) const
{
    if (is_complex())
    {
        vector<TypePtr> instantiated_subtypes;
        instantiated_subtypes.reserve(subtypes_.size());
        for (const TypePtr& subtype : subtypes_)
        {
            instantiated_subtypes.push_back(
                std::make_unique<Type>(
                    subtype->instantiate_template_type(template_type)
                )
            );
        }

        return Type{std::move(instantiated_subtypes)};
    }
    else
    {
        return name_ == "T" ? template_type : *this;
    }
}


string Type::complex_name() const
{
    string str = "<";
    for (const TypePtr& subtype : subtypes_)
    {
        str += subtype->str();
        str += ", ";
    }
    str.pop_back();
    str.pop_back();
    str += ">";
    return str;
}
