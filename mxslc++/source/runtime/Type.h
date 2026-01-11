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
public:
    static const Type Bool, Int, Float, String;

    Type();
    Type(string name);
    Type(const Token& token);
    explicit Type(const basic_t& value);
    explicit Type(vector<Type> subtypes);
    explicit Type(const vector<string>& subtypes);

    [[nodiscard]] Type instantiate_template_types(const Type& template_type) const;

    [[nodiscard]] bool is_complex() const { return not subtypes_.empty(); }
    [[nodiscard]] size_t subtype_count() const { return subtypes_.size(); }
    [[nodiscard]] const Type& subtype(const size_t i) const { return subtypes_.at(i); }
    [[nodiscard]] const vector<Type>& subtypes() const { return subtypes_; }
    [[nodiscard]] const string& str() const { return name_; }

    bool operator==(const Type& other) const { return str() == other.str(); }
    bool operator==(const string& other) const { return str() == other; }

    bool operator!=(const Type& other) const { return not (*this == other); }
    bool operator!=(const string& other) const { return not (*this == other); }

    template<typename T>
    static optional<T> instantiate_template_types(const optional<T>& src, const Type& template_type)
    {
        if (src)
            return src->instantiate_template_types(template_type);
        return std::nullopt;
    }

    template<typename T>
    static vector<T> instantiate_template_types(const vector<T>& src, const Type& template_type)
    {
        vector<T> dst;
        dst.reserve(src.size());
        for (const T& t : src)
            dst.push_back(t.instantiate_template_types(template_type));
        return dst;
    }

    template<typename T>
    static vector<unique_ptr<T>> instantiate_template_types(const vector<unique_ptr<T>>& src, const Type& template_type)
    {
        vector<unique_ptr<T>> dst;
        dst.reserve(src.size());
        for (const unique_ptr<T>& t : src)
            dst.push_back(t->instantiate_template_types(template_type));
        return dst;
    }

private:
    [[nodiscard]] string complex_name() const;

    string name_;
    vector<Type> subtypes_;
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
