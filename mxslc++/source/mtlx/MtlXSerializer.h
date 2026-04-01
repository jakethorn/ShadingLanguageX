//
// Created by jaket on 04/01/2026.
//

#ifndef FENNEC_MTLXSERIALIZER_H
#define FENNEC_MTLXSERIALIZER_H

#include <MaterialXCore/Document.h>
#include "utils/common.h"

class ArgumentList;
class Function;

class MtlXSerializer
{
public:
    MtlXSerializer() : MtlXSerializer{mx::createDocument()} { }
    explicit MtlXSerializer(mx::DocumentPtr doc) : doc_{std::move(doc)}
    {
        graphs_.push_back(doc_);
    }

    [[nodiscard]] ValuePtr write_node(const Function& func, const ArgumentList& args) const;
    void write_node_def_graph(const Function& func) const;
    void nonlocal_variable_access(const Variable& var);
    void nonlocal_variable_assignment(const Variable& var);
    void save(const fs::path& filepath) const;

private:
    mx::NodeDefPtr write_node_def(const Function& func) const;
    void write_node_graph(const Function& func, const mx::NodeDefPtr& node_def) const;

    string node_def_name(const Function& func) const;
    string node_graph_name(const Function& func) const;

    void enter_node_graph(const mx::NodeGraphPtr& node_graph) const;
    void exit_node_graph() const;
    const mx::GraphElementPtr& graph() const { return graphs_.back(); }

    mx::DocumentPtr doc_;
    mutable vector<mx::GraphElementPtr> graphs_;
};

#endif //FENNEC_MTLXSERIALIZER_H
