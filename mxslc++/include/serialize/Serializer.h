//
// Created by jaket on 04/01/2026.
//

#ifndef FENNEC_MTLXSERIALIZER_H
#define FENNEC_MTLXSERIALIZER_H

#include <stack>

#include <MaterialXCore/Document.h>

#include "common.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::runtime
{
    class ArgumentList;
    class AttributeList;
}

namespace mxslc::serialize
{
    class Serializer : protected runtime_utils::RuntimeAware
    {
    public:
        Serializer() : Serializer{mx::createDocument()} { }
        explicit Serializer(mx::DocumentPtr doc) : doc_{std::move(doc)} { }

        bool reduce_graph() const { return reduce_graph_; }

        void set_version(const string& version);
        void set_reduce_graph(bool value);
        void set_single_use_as_nodegraph(bool value);

        // Called once all code has been serialized. Uses the per-function usage
        // count collected during serialization to decide whether each function is
        // written as a bare nodegraph (single-use) or as a nodedef (reused), since
        // a nodegraph cannot be referenced and reused more than once in MaterialX.
        void finalize();

        // Records a use of a function in the code being serialized.
        void count_use(const FuncPtr& func) const;

        void begin_comptime(bool is_comptime) const;
        bool end_comptime() const;

        VarPtr write_node(const FuncPtr& func, const ArgumentList& args, const AttributeList& attrs) const;
        VarPtr write_node(const VarPtr& instance, const FuncPtr& func, const ArgumentList& args, const AttributeList& attrs) const;

        void write_node_def_graph(const FuncPtr& func) const;
        void write_node_def_graph(const FuncPtr& func, const AttributeList& attrs) const;

        ValuePtr write_node_def_input(const VarPtr& var) const;
        void write_node_def_output(const VarPtr& var, const ValuePtr& value) const;

        mx::DocumentPtr document() const { return doc_; }
        string xml() const;
        void save(const fs::path& dst_path) const;

    private:
        mx::NodeDefPtr write_node_def(const FuncPtr& func) const;
        mx::NodeGraphPtr write_node_graph(const FuncPtr& func, const mx::NodeDefPtr& node_def) const;

        void add_instance_to_scope(const FuncPtr& func, const mx::NodeDefPtr& node_def) const;
        VarPtr copy_instance(const FuncPtr& func) const;
        void update_instance(const FuncPtr& func, const mx::NodeGraphPtr& node_graph, const VarPtr& original_instance) const;

        void write_node_input(const mx::NodePtr& node, const string& input_name, const VarPtr& var) const;
        void write_node_input(const mx::NodePtr& node, const string& input_name, const VarPtr& var, const AttributeList& attrs) const;
        void write_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name, const VarPtr& var) const;
        void write_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name, const VarPtr& var, const AttributeList& attrs) const;
        void write_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name, const TypePtr& type) const;
        void write_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name, const VarPtr& var) const;
        void write_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name, const VarPtr& var, const AttributeList& attrs) const;

        string node_def_name(const FuncPtr& func) const;
        string node_graph_name(const FuncPtr& func) const;

        mx::DocumentPtr doc_;
        bool reduce_graph_{true};

        bool single_use_as_nodegraph_{true};
        mutable unordered_map<const Function*, size_t> usage_;
        mutable vector<FuncPtr> deferred_;

        mutable std::stack<bool> comptime_scope_{{false}};
        mutable bool comptime_violated_{false};
    };
}

#endif //FENNEC_MTLXSERIALIZER_H
