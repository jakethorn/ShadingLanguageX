//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILER_H
#define MXSLC_DECOMPILER_H

#include <MaterialXCore/Document.h>

#include "utils/common.h"

namespace mxslc
{
    string decompile_to_string(const fs::path& src_path);
    string decompile_to_string(const string& source);
    string decompile_to_string(const mx::DocumentPtr& document);

    fs::path decompile_to_file(const fs::path& src_path);
    fs::path decompile_to_file(const fs::path& src_path, const fs::path& dst_path);
    fs::path decompile_to_file(const string& source, const fs::path& dst_path);
    fs::path decompile_to_file(const mx::DocumentPtr& document, const fs::path& dst_path);

    class Decompiler
    {
    public:
        explicit Decompiler(const fs::path& src_path);
        explicit Decompiler(const string& source);
        explicit Decompiler(mx::DocumentPtr document);

        string decompile_document();
        string decompile_node(const string& node_name);
        string decompile_node(const mx::NodePtr& node);
        string decompile_node_graph(const mx::NodeGraphPtr& node_graph);

    private:
        string decompile_outputs(const vector<mx::OutputPtr>& outputs);
        string decompile_value(const mx::ValuePtr& value);
        string decompile_interface_name(const string& interface_name);
        string decompile_node_name_and_output(const string& node_name, const string& output);
        string decompile_node_name(const string& node_name);
        string decompile_node_input(const mx::InputPtr& input);
        string decompile_node_inputs(const vector<mx::InputPtr>& inputs);
        string decompile_node_def_input(const mx::InputPtr& input);
        string decompile_node_def_inputs(const vector<mx::InputPtr>& inputs);

        mx::DocumentPtr document_;
        string code_;
        unordered_set<string> decompiled_nodes_;
    };
}

#endif //MXSLC_DECOMPILER_H
