//
// Created by jaket on 07/12/2025.
//

#include "Type.h"

#include <cassert>

#include "CompileError.h"
#include "Token.h"
#include "utils/template_utils.h"

Type::Type() : Type{"__not_a_type__"s} { }
Type::Type(string name) : name_{std::move(name)} { }
Type::Type(const Token& token) : Type{token.lexeme()} { }

namespace
{
    string to_type(const basic_t& value)
    {
        if (std::holds_alternative<bool>(value))
            return "boolean"s;
        if (std::holds_alternative<int>(value))
            return "integer"s;
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

Type::Type(vector<Type> subtypes)
{
    assert(not subtypes.empty());

    if (subtypes.size() == 1)
    {
        subtypes_ = std::move(subtypes[0].subtypes_);
        name_ = subtypes[0].name_;
    }
    else
    {
        subtypes_ = std::move(subtypes);
        name_ = complex_name();
    }
}

Type::Type(const vector<string>& subtypes) : Type{cast_vector<Type>(subtypes)}
{

}

Type Type::instantiate_template_types(const Type& template_type) const
{
    if (is_complex())
    {
        vector<Type> instantiated;
        instantiated.reserve(subtypes_.size());
        for (const Type& subtype : subtypes_)
            instantiated.push_back(subtype.instantiate_template_types(template_type));
        return Type{std::move(instantiated)};
    }
    else
    {
        return name_ == "T" ? template_type : *this;
    }
}

string Type::complex_name() const
{
    string str = "<";
    for (size_t i = 0; i < subtypes_.size(); ++i)
    {
        str += subtypes_[i].name_;
        if (i < subtypes_.size() - 1)
            str += ", ";
    }
    str += ">";
    return str;
}
