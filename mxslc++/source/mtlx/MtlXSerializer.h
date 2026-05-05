//
// Created by jaket on 04/01/2026.
//

#ifndef FENNEC_MTLXSERIALIZER_H
#define FENNEC_MTLXSERIALIZER_H

#include <MaterialXCore/Document.h>

#include "utils/common.h"

class ArgumentList;
class AttributeList;

class MtlXSerializer
{
public:
    MtlXSerializer() : MtlXSerializer{mx::createDocument()} { }
    explicit MtlXSerializer(mx::DocumentPtr doc) : doc_{std::move(doc)} { }

    VarPtr write_node(const FuncPtr& func, const ArgumentList& args, const AttributeList& attrs) const;

    void write_node_def_graph(const FuncPtr& func, const AttributeList& attrs) const;

    ValuePtr write_node_def_input(const VarPtr& var) const;
    void write_node_def_output(const VarPtr& var, const ValuePtr& value) const;

    mx::DocumentPtr document() const { return doc_; }
    string xml() const;
    void save(const fs::path& filepath) const;

private:
    mx::NodeDefPtr write_node_def(const FuncPtr& func) const;
    void write_node_graph(const FuncPtr& func, const mx::NodeDefPtr& node_def) const;

    void write_node_input(const mx::NodePtr& node, const string& input_name, const VarPtr& var) const;
    void write_node_input(const mx::NodePtr& node, const string& input_name, const VarPtr& var, const AttributeList& attrs) const;
    void write_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name, const VarPtr& var) const;
    void write_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name, const VarPtr& var, const AttributeList& attrs) const;
    void write_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name, const VarPtr& var, const AttributeList& attrs) const;

    string node_def_name(const FuncPtr& func) const;
    string node_graph_name(const FuncPtr& func) const;

    mx::DocumentPtr doc_;
};

#endif //FENNEC_MTLXSERIALIZER_H
