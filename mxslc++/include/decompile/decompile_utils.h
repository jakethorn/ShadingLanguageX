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

    // nodes of temporary values are named var__<n> by the compiler
    bool is_temporary_name(const string& name);
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
    bool has_literal_syntax(const string& type_name);
    // true if the value is zero, false or an empty string
    bool is_zero(const mx::ValuePtr& value);
    bool is_zero_value(const mx::ValueElementPtr& element);

    // the channel of an output of a separate node, e.g., 'y' for outy
    optional<char> swizzle_channel(const string& output_name);
    bool is_color_type(const string& type_name);
    // the fields of structs without names are indexed, e.g., the 0 of out__0
    bool is_index(const string& field_name);
    bool is_connected(const mx::InputPtr& input);
}

#endif //MXSLC_DECOMPILE_UTILS_H
