//
// Created by jaket on 04/01/2026.
//

#ifndef FENNEC_MTLXSERIALIZER_H
#define FENNEC_MTLXSERIALIZER_H

#include <MaterialXCore/Document.h>
#include "utils/common.h"

struct GraphFunction
{
    mx::GraphElementPtr graph;
    FuncPtr func;
};

class MtlXSerializer
{
public:
    MtlXSerializer() : MtlXSerializer{mx::createDocument()} { }
    explicit MtlXSerializer(mx::DocumentPtr doc) : doc_{std::move(doc)}
    {
        graphs_.push_back(GraphFunction{doc_, nullptr});
    }

    ValuePtr write_node(const FuncPtr& func, const unordered_map<string, ValuePtr>& args) const;
    ValuePtr write_node_def_input(const ConstVarPtr& var) const;
    void write_node_def_output(const VarPtr& var, const ValuePtr& value) const;
    void write_node_def_graph(const FuncPtr2& func) const;
    void save(const fs::path& filepath) const;

private:
    mx::NodeDefPtr write_node_def(const FuncPtr2& func) const;
    void write_node_graph(const FuncPtr2& func, const mx::NodeDefPtr& node_def) const;

    string node_def_name(const FuncPtr2& func) const;
    string node_graph_name(const FuncPtr2& func) const;

    void enter_node_graph(const mx::NodeGraphPtr& node_graph, const FuncPtr& func) const;
    void exit_node_graph() const;
    bool in_node_graph() const { return graphs_.size() > 1; }
    const mx::GraphElementPtr& graph() const { return graphs_.back().graph; }
    mx::NodeGraphPtr node_graph() const;
    FuncPtr graph_function() const { return graphs_.back().func; }

    mx::DocumentPtr doc_;
    mutable vector<GraphFunction> graphs_;
};

#endif //FENNEC_MTLXSERIALIZER_H
