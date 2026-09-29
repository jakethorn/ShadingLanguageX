//
// Created by jaket on 28/09/2026.
//

// Graphs compiled with decompile hints are edited, e.g., as they would be in a node graph editor, and then decompiled.
// The code must always compile to the edited graph, and the parts of the code that were not edited should still be
// recreated from the hints.

#include "gtest/gtest.h"
#include <functional>
#include <string>
#include <vector>
#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "utils/data_utils.h"
#include "utils/graph_utils.h"

using std::string;
using std::vector;

namespace
{
    struct EditCase
    {
        string name;
        string source;
        std::function<void(const mx::DocumentPtr&)> edit;
        // code that must be part of the decompiled code
        vector<string> expected;
        vector<string> unexpected;
    };

    mx::NodePtr find_node(const mx::DocumentPtr& doc, const std::function<bool(const mx::NodePtr&)>& predicate)
    {
        for (const mx::NodePtr& node : doc->getNodes())
        {
            if (predicate(node))
                return node;
        }
        throw std::runtime_error{"node not found"};
    }

    bool has_value(const mx::NodePtr& node, const string& input_name, const string& value)
    {
        const mx::InputPtr input = node->getInput(input_name);
        return input and input->getNodeName().empty() and input->getValueString() == value;
    }

    void remove_hints(const mx::ElementPtr& element)
    {
        // the names are copied, because removing an attribute changes them
        const vector<string> names = element->getAttributeNames();
        for (const string& name : names)
        {
            if (name.rfind("dc_", 0) == 0)
                element->removeAttribute(name);
        }
    }

    void rename_node(const mx::DocumentPtr& doc, const string& old_name, const string& new_name)
    {
        doc->getNode(old_name)->setName(new_name);
        for (const mx::NodePtr& node : doc->getNodes())
        {
            for (const mx::InputPtr& input : node->getInputs())
            {
                if (input->getNodeName() == old_name)
                    input->setNodeName(new_name);
            }
        }
    }

    vector<EditCase> edit_cases()
    {
        return {
            {
                "unedited",
                "hints/inline_functions.mxsl",
                [](const mx::DocumentPtr&) { },
                {"inline float sq(float v)", "float b = sq(a + 1.0) + sq(0.5);"},
                {}
            },
            {
                "constant_in_one_call",
                "hints/inline_functions.mxsl",
                [](const mx::DocumentPtr& doc) {
                    // sq(0.5) is v * v with both inputs 0.5
                    const mx::NodePtr node = find_node(doc, [](const mx::NodePtr& n) { return has_value(n, "in1", "0.5"); });
                    node->getInput("in1")->setValueString("0.7");
                },
                {"inline float sq(float v)", "float b = sq(a + 1.0) + 0.7 * 0.5;"},
                {}
            },
            {
                "node_inserted",
                "hints/inline_functions.mxsl",
                [](const mx::DocumentPtr& doc) {
                    const mx::NodePtr sin = doc->addNode("sin", "my_sin", "float");
                    sin->setConnectedNode("in", doc->getNode("a"));
                    const mx::NodePtr add = find_node(doc, [](const mx::NodePtr& n) { return n->getCategory() == "add" and n->getInput("in1")->getNodeName() == "a"; });
                    add->setConnectedNode("in1", sin);
                },
                {"float my_sin = sin(a);", "float b = sq(my_sin + 1.0) + sq(0.5);"},
                {}
            },
            {
                "call_records_removed",
                "hints/inline_functions.mxsl",
                [](const mx::DocumentPtr& doc) {
                    const vector<string> names = doc->getAttributeNames();
                    for (const string& name : names)
                    {
                        if (name.rfind("dc_c", 0) == 0)
                            doc->removeAttribute(name);
                    }
                },
                {"float a = "},
                {"inline float sq"}
            },
            {
                "loop_iteration_edited",
                "hints/for_loops.mxsl",
                [](const mx::DocumentPtr& doc) {
                    const mx::NodePtr node = find_node(doc, [](const mx::NodePtr& n) {
                        return n->getCategory() == "randomfloat" and has_value(n, "in", "2") and has_value(n, "seed", "1");
                    });
                    node->getInput("seed")->setValueString("5");
                },
                {"randomfloat(2, seed = 5)", "for (int octave from 0 to 3)", "for (float w from {0.25, 0.5, 1.0})"},
                {"for (int i from 1 to 4)"}
            },
            {
                "old_value_referenced",
                "hints/mutable_variables.mxsl",
                [](const mx::DocumentPtr& doc) {
                    // the value of x before it is multiplied by 2
                    const mx::NodePtr first_x = find_node(doc, [](const mx::NodePtr& n) { return n->getName() == "x"; });
                    const mx::NodePtr sin = doc->addNode("sin", "old", "float");
                    sin->setConnectedNode("in", first_x);
                    doc->addNode("constant", "result", "float")->setConnectedNode("value", sin);
                },
                {"float x_copy = x;", "x += p.y;", "float old = sin(x_copy);"},
                {}
            },
            {
                "compile_time_value_edited",
                "hints/basic002.mxsl",
                [](const mx::DocumentPtr& doc) {
                    // x is a compile-time value, so the input holds its value
                    const mx::NodePtr node = find_node(doc, [](const mx::NodePtr& n) { return has_value(n, "in1", "1"); });
                    node->getInput("in1")->setValueString("5");
                },
                {"int x = foo();", "int z = 5 + y;"},
                {}
            },
            {
                "node_renamed",
                "hints/constants.mxsl",
                [](const mx::DocumentPtr& doc) { rename_node(doc, "theta", "angle2"); },
                {"float angle2 = uv.y * PI * 2.0;", "vec2 dir = vec2{cos(angle2), sin(angle2)};"},
                {}
            },
            {
                "library_call_edited",
                "hints/interior_mapping.mxsl",
                [](const mx::DocumentPtr& doc) {
                    // -origin is compiled to invert(origin, 0)
                    const mx::NodePtr node = find_node(doc, [](const mx::NodePtr& n) { return n->getCategory() == "invert"; });
                    node->getInput("amount")->setValueString("0.5");
                },
                {"inline vec3 raycast(vec3 origin, vec3 view_dir)", "invert(origin, 0.5)"},
                {}
            },
            {
                "partial_hints",
                "hints/interior_mapping.mxsl",
                [](const mx::DocumentPtr& doc) {
                    const vector<mx::NodePtr> nodes = doc->getNodes();
                    for (size_t i = 0; i < nodes.size(); i += 2)
                    {
                        remove_hints(nodes[i]);
                        for (const mx::InputPtr& input : nodes[i]->getInputs())
                            remove_hints(input);
                    }
                },
                {"material parallax_mat = surfacematerial(surface);"},
                {}
            },
            {
                "hints_removed",
                "hints/for_loops.mxsl",
                [](const mx::DocumentPtr& doc) {
                    remove_hints(doc);
                    for (const mx::ElementPtr& element : doc->traverseTree())
                        remove_hints(element);
                },
                {},
                {"for ("}
            },
        };
    }
}

using edited_graph_tests = testing::TestWithParam<EditCase>;

TEST_P(edited_graph_tests, decompiled_edited_graph_compiles_to_edited_graph)
{
    const EditCase& edit_case = GetParam();

    mxslc::CompileOptions opts;
    opts.reduce_graph = false;
    opts.decompile_hints = true;
    const mx::DocumentPtr document = mxslc::compile_to_document(get_test_data("roundtrip") / edit_case.source, opts);
    edit_case.edit(document);

    const string decompiled = mxslc::decompile_to_string(document);

    opts.decompile_hints = false;
    const mx::DocumentPtr recompiled = mxslc::compile_to_document(decompiled, opts);
    const vector<string> differences = GraphComparator::differences(document, recompiled);
    EXPECT_TRUE(differences.empty()) << "different elements: " << testing::PrintToString(differences) << "\n" << decompiled;

    for (const string& expected : edit_case.expected)
        EXPECT_NE(decompiled.find(expected), string::npos) << "missing: " << expected << "\n" << decompiled;
    for (const string& unexpected : edit_case.unexpected)
        EXPECT_EQ(decompiled.find(unexpected), string::npos) << "unexpected: " << unexpected << "\n" << decompiled;
}

INSTANTIATE_TEST_SUITE_P(
    decompiler,
    edited_graph_tests,
    testing::ValuesIn(edit_cases()),
    [](const testing::TestParamInfo<EditCase>& info) {
        return info.param.name;
    }
);
