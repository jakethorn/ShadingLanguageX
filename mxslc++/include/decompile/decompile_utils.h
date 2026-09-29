//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_DECOMPILE_UTILS_H
#define MXSLC_DECOMPILE_UTILS_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/Code.h"

namespace mxslc::decompile_utils
{
    using decompile::Code;

    // the ShadingLanguageX name of a MaterialX type, e.g., vec3 for vector3
    string type_alias(const string& type_name);
    // true if the name is a MaterialX type that can be used in ShadingLanguageX, e.g., float or surfaceshader
    bool is_type_name(const string& name);
    // true if values of the type can be written as literals, e.g., `1.0`, `"text"` or `vec3{1.0, 2.0, 3.0}`
    bool has_literal_syntax(const string& type_name);
    // the channels of vector and color types, e.g., "xyz" for vector3 and "rgba" for color4, otherwise empty
    string swizzle_channels(const string& type_name);
    bool is_color_type(const string& type_name);

    // nodes of temporary values are named var__<n> by the compiler, and those of values assigned to a variable after its
    // definition are named var__<variable>__<n>
    bool is_temporary_name(const string& name);
    // the variable of a value assigned to it after its definition, e.g., x for var__x__2
    optional<string> assigned_variable(const string& node_name);
    bool is_valid_identifier(const string& name);
    // a valid identifier that is as close to the name as possible, e.g., node_1 for node-1
    string to_identifier(const string& name);

    // the attributes of an element that are not expressed by the code, e.g., `@uiname "Color"`, which are attributes of
    // the child if it is named, e.g., `@out.doc "..."` for the output of a node def
    vector<string> user_attributes(const mx::ElementPtr& element, const string& child_name = "");

    // e.g., `1.0`, `"text"`, `vec3{1.0, 2.0, 3.0}`, `vec3{}` if all components are zero and `color3{1.0}` if all
    // components are the same, or nothing if the value has no literal syntax
    optional<Code> literal(const mx::ValuePtr& value);
    // the value of an input, output or parameter, which can also be the default of its type, e.g., `default(surfaceshader)`
    optional<Code> literal(const mx::ValueElementPtr& element);
    // true if the value is zero, false or an empty string
    bool is_zero(const mx::ValuePtr& value);
    bool is_zero_value(const mx::ValueElementPtr& element);

    // the channel of an output of a separate node, e.g., 'y' for outy
    optional<char> swizzle_channel(const string& output_name);
    // the fields of structs without names are indexed, e.g., the 0 of out__0
    bool is_index(const string& field_name);
    bool is_connected(const mx::InputPtr& input);
}

#endif //MXSLC_DECOMPILE_UTILS_H
