//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILER_H
#define MXSLC_DECOMPILER_H

#include <MaterialXCore/Document.h>

#include "common.h"

namespace mxslc::decompile
{
    class Decompiler
    {
    public:
        explicit Decompiler(const fs::path& src_path);
        explicit Decompiler(const string& source);
        explicit Decompiler(const mx::DocumentPtr& document);

        string decompile_document();
        string decompile_node(const string& node_name, bool with_dependencies = false);
        string decompile_node(const mx::NodePtr& node, bool with_dependencies = false);
        string decompile_node_def(const string& node_def_name, bool with_dependencies = false);
        string decompile_node_def(const mx::NodeDefPtr& node_def, bool with_dependencies = false);
        string decompile_node_graph(const string& node_graph_name, bool with_dependencies = false);
        string decompile_node_graph(const mx::NodeGraphPtr& node_graph, bool with_dependencies = false);

    private:
        void load_data_library(const vector<fs::path>& search_directories) const;

        // a copy of the source document with the MaterialX libraries of its version set as its data library
        mx::DocumentPtr document_;
    };
}

#endif //MXSLC_DECOMPILER_H
