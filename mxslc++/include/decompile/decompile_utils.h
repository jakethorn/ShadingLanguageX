//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_DECOMPILE_UTILS_H
#define MXSLC_DECOMPILE_UTILS_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "Token.h"
#include "runtime/AttributeList.h"

namespace mxslc::decompile_utils
{
    // vector3 -> vec3, boolean -> bool, etc.
    string type_alias(const string& type_name);
    TypePtr create_type_from(const string& type_name);

    // nodes named var__N were created for temporaries, i.e., sub-expressions, by the compiler
    bool is_temporary_name(const string& name);

    // true if the name is a valid ShadingLanguageX identifier that is not reserved
    bool is_valid_identifier(const string& name);
    // converts a MaterialX name into a valid identifier, e.g., `default` -> `default_`
    string to_identifier(const string& name);

    // attributes that are not otherwise expressed by the decompiled code, e.g., doc or colorspace
    AttributeList user_attributes(const mx::ElementPtr& element, const string& child_name = "");

    ExprPtr create_literal(const mx::ValuePtr& value);
    ExprPtr create_identifier(const string& name);
    Token create_symbol(const string& symbol);

    // true if values of the type can be written as literals, e.g., 1.0 or vec3{1.0, 0.0, 0.0}, but not surfaceshader
    bool has_literal_syntax(const string& type_name);

    // true if the value is the default value of its type, e.g., 0.0 or vec3{0.0, 0.0, 0.0}
    // e.g., {"x", "0"} for "x__0" and "__"
    vector<string> split_string(const string& str, const string& delimiter);
    bool is_zero_value(const mx::ValueElementPtr& element);
    // true if the value is zero, false, or an empty string
    bool is_zero(const mx::ValuePtr& value);

    // outx -> x, outr -> r, etc.
    optional<char> swizzle_channel(const string& output_name);
    bool is_color_type(const string& type_name);
    // true if the input is connected to a node, a node graph or an interface input
    bool is_connected(const mx::InputPtr& input);

    // sets whether the type of the expression being created is known from its context, until it is destroyed
    class TypedContext
    {
    public:
        TypedContext(bool& flag, const bool value) : flag_{flag}, saved_{flag} { flag_ = value; }
        ~TypedContext() { flag_ = saved_; }
        TypedContext(const TypedContext&) = delete;
        TypedContext& operator=(const TypedContext&) = delete;

    private:
        bool& flag_;
        bool saved_;
    };
}

#endif //MXSLC_DECOMPILE_UTILS_H
