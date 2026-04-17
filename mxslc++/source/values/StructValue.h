//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_STRUCTVALUE_H
#define MXSLC_STRUCTVALUE_H

//#include "Value.h"
//#include "mtlx/mtlx_utils.h"
//#include "utils/str_utils.h"
//
//class StructValue final : public Value
//{
//public:
//    explicit StructValue(vector<ValuePtr> values, TypeInfoPtr type)
//        : Value{std::move(type)}, values_{std::move(values)}
//    {
//        assert(not values_.empty());
//        mx::GraphElementPtr parent_graph = nullptr;
//        for (const ValuePtr& value : values_)
//        {
//            if (parent_graph)
//                assert(value->parent_graph() == nullptr or value->parent_graph() == parent_graph);
//            else if (value->parent_graph())
//                parent_graph = value->parent_graph();
//        }
//    }
//
//    ValuePtr subvalue(const size_t i) const override { return values_.at(i); }
//    void set_subvalue(const size_t i, ValuePtr value) override { values_[i] = std::move(value); }
//
//    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
//    {
//        for (size_t i = 0; i < values_.size(); ++i)
//            values_[i]->set_as_node_input(node, port_name(input_name, i));
//    }
//
//    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override
//    {
//        for (size_t i = 0; i < values_.size(); ++i)
//            values_[i]->set_as_node_graph_output(node_graph, port_name(output_name, i));
//    }
//
//    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override
//    {
//        for (size_t i = 0; i < values_.size(); ++i)
//            values_[i]->set_as_node_def_input(node_def, port_name(input_name, i));
//    }
//
//    string str() const override
//    {
//        string result;
//        for (size_t i = 0; i < values_.size(); ++i)
//        {
//            result += "index " + ::str(i) + ": " + values_[i]->str();
//            if (i < values_.size() - 1)
//                result += "\n";
//        }
//
//        return result;
//    }
//
//    mx::GraphElementPtr parent_graph() const override
//    {
//        return values_[0]->parent_graph();
//    }
//
//private:
//    vector<ValuePtr> values_;
//};

#endif //MXSLC_STRUCTVALUE_H
