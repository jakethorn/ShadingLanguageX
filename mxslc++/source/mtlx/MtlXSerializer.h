//
// Created by jaket on 04/01/2026.
//

#ifndef FENNEC_MTLXSERIALIZER_H
#define FENNEC_MTLXSERIALIZER_H

#include <MaterialXCore/Document.h>

#include "utils/common.h"
#include "runtime/Serializer.h"

class MtlXSerializer final : public Serializer
{
public:
    MtlXSerializer() : MtlXSerializer{mx::createDocument()} { }
    MtlXSerializer(mx::DocumentPtr doc) : doc_{std::move(doc)}
    {
        graphs_.push_back(doc_);
    }

    [[nodiscard]] ValuePtr write_function_call(const Function& func, const ArgumentList& args) const override;
    void write_function(const Function& func) const override;
    void save(const fs::path& filepath) const override;

private:
    void write_node_def(const Function& func) const;
    void write_node_graph(const Function& func) const;

    void enter_node_graph(const mx::NodeGraphPtr& node_graph) const;
    void exit_node_graph() const;
    const mx::GraphElementPtr& graph() const { return graphs_.back(); }

    mx::DocumentPtr doc_;
    mutable vector<mx::GraphElementPtr> graphs_;
};

#endif //FENNEC_MTLXSERIALIZER_H
