//
// Created by jaket on 28/09/2026.
//

#ifndef MXSLC_DECOMPILE_UTILS_H
#define MXSLC_DECOMPILE_UTILS_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "decompile/SourceCode.h"

namespace mxslc::decompile_utils
{
    using decompile::ExpressionCode;

    // the ShadingLanguageX name of a MaterialX type, e.g., vec3 for vector3
    string get_type_alias(const string& type_name);
    string get_type_alias(const mx::TypedElementPtr& elem);
    // true if the name is a MaterialX type that can be used in ShadingLanguageX, e.g., float or surfaceshader
    bool is_type_name(const string& name);
    // true if values of the type can be written as literals, e.g., `1.0`, `"text"` or `vec3{1.0, 2.0, 3.0}`
    bool has_literal_syntax(const string& type_name);
    // the channels of vector and color types, e.g., "xyz" for vector3 and "rgba" for color4, otherwise empty
    string get_swizzle_channels(const string& type_name);
    bool is_color_type(const string& type_name);
    // the template type that selects a node def of the libraries, which is the postfix of its name if it is a type,
    // e.g., float for ND_noise2d_float, otherwise it has none, e.g., ND_fractal2d_vector3FA, as in the compiler
    string get_library_template_type(const mx::NodeDefPtr& node_def);

    bool is_valid_identifier(const string& name);
    // a valid identifier that is as close to the name as possible, e.g., node_1 for node-1
    string make_identifier(const string& name);

    // the attributes of an element that are not expressed by the code, e.g., `@uiname "Color"`, which are attributes of
    // the child if it is named, e.g., `@out.doc "..."` for the output of a node def
    vector<string> get_user_attributes(const mx::ElementPtr& element, const string& child_name = "");

    // e.g., `1.0`, `"text"`, `vec3{1.0, 2.0, 3.0}`, `vec3{}` if all components are zero and `color3{1.0}` if all
    // components are the same, or nothing if the value has no literal syntax
    optional<ExpressionCode> format_value(const mx::ValuePtr& value);
    // the value of an input, output or parameter, which can also be the default of its type, e.g., `default(surfaceshader)`
    optional<ExpressionCode> format_value(const mx::ValueElementPtr& element);

    // the fields of structs without names are indexed, e.g., the 0 of out__0
    bool is_index(const string& field_name);
    // the return type of a function with these outputs, e.g., `float`, `{float a, vec2 b}` or `{float, vec2}` if its
    // fields have no names
    string get_return_type(const vector<mx::OutputPtr>& outputs);
    // the names that the compiler gives the outputs of a return value with these outputs, which are out, or
    // out__<field> for the fields of a struct, e.g., out__outx for the output outx of ND_separate3_vector3
    vector<string> get_compiled_return_output_names(const vector<mx::OutputPtr>& outputs);

    // e.g., separate3
    bool is_separate(const mx::NodePtr& node);
    // e.g., combine3
    bool is_combine(const mx::NodePtr& node);
    // the number of outputs of a separate node or inputs of a combine node, otherwise zero
    size_t get_channel_count(const mx::NodePtr& node);
    // the channel of an output of a separate node, e.g., 'y' for outy
    optional<char> get_separate_channel(const string& output_name);

    // true if the input is connected to a node, a node graph or an interface input
    bool is_connected(const mx::InputPtr& input);
    // true if the input has a boolean value, rather than a connection, that is the expected value
    bool has_bool_value(const mx::InputPtr& input, bool expected);
    // true if the input has a float value, rather than a connection, that is the expected value
    bool has_float_value(const mx::InputPtr& input, float expected);
    // the value of the input if it is an integer, rather than a connection
    optional<int> get_int_value(const mx::InputPtr& input);
    // the components of a numeric value, e.g., {1, 2, 3} for "1, 2, 3", empty if the value is not numeric
    vector<float> get_float_components(const mx::ValuePtr& value);
    // true if the value is zero, false or an empty string
    bool is_zero_value(const mx::ValuePtr& value);
    bool is_zero_value(const mx::ValueElementPtr& element);

    // the variable of a value that is assigned to it after its definition, e.g., x for var__x__2
    optional<string> get_assigned_variable_name(const string& node_name);
    // true if the node is not named after the definition of a variable, i.e., it is a temporary value, var__<n>, or a
    // value that is assigned to a variable after its definition, var__<variable>__<n>
    bool is_generated_node_name(const string& name);
    // true if the node is named like MaterialX names nodes that are added without a name, i.e., node<n>, e.g., the dot
    // nodes that the compiler passes parameters through to the outputs of a function
    bool is_default_node_name(const string& name);

    // the outputs of a node def that are its return value, e.g., out, or out__<field> if it returns a struct, as opposed
    // to its out parameters and nonlocal variables
    bool is_return_output(const mx::OutputPtr& output);

    // MaterialX looks up elements by name in the data library before the document, e.g., Document::getNodeDef and
    // NodeGraph::getNodeDef, so an element of the document is hidden by a library element with the same name, e.g., the
    // node def ND_invert_float of the templated function `T invert<float, vec3>(T in)`, and its lists of elements also
    // contain the library elements, e.g., Document::getNodeGraphs. These find the elements of the document instead.
    // Document::getNode even returns null if a library element of another type has the name, e.g., for a node named
    // distance, because the libraries define the unit type distance.

    // the node def or node graph of the document with the name, or null if the document has none, even if the MaterialX
    // libraries have one
    mx::NodeDefPtr get_document_node_def(const mx::DocumentPtr& document, const string& name);
    mx::NodeGraphPtr get_document_node_graph(const mx::DocumentPtr& document, const string& name);
    // the node defs or node graphs of the document, without those of the MaterialX libraries
    vector<mx::NodeDefPtr> get_document_node_defs(const mx::DocumentPtr& document);
    vector<mx::NodeGraphPtr> get_document_node_graphs(const mx::DocumentPtr& document);
    // the node def that the node graph implements, named by the node graph or by an implementation element of its
    // document, which is the node def of its document if it has one with the name, otherwise a node def of the MaterialX
    // libraries, or null if the node graph does not implement a node def
    mx::NodeDefPtr get_node_def(const mx::NodeGraphPtr& node_graph);
    // the node graph that implements the node def, from the document of the node def, or null if it has none, e.g., it is
    // implemented in GLSL
    mx::NodeGraphPtr get_node_graph(const mx::NodeDefPtr& node_def);
    // the node of the graph, the document or a node graph, with the name, or null if it has none
    mx::NodePtr get_node(const mx::GraphElementPtr& graph, const string& name);

    // Sorts the elements of the document, and the nodes of its node graphs, into the order of the code that they could be
    // compiled from, where each element comes after the elements that it depends on, e.g., a node after the node def of
    // the function that it calls, and the values of a variable, e.g., x, var__x__1 and var__x__2, are in the order that
    // they were assigned. Elements keep their order where possible, so the documents of the compiler are not changed.
    void sort_by_dependencies(const mx::DocumentPtr& document);

}

#endif //MXSLC_DECOMPILE_UTILS_H
