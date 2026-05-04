//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VALUE_H
#define FENNEC_VALUE_H

#include "utils/common.h"

class Value
{
public:
    explicit Value(TypePtr type);
    virtual ~Value() = default;

    TypePtr type() { return type_; }

    virtual void set_as_node_input(const mx::InputPtr& input) const { }
    virtual void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const { }
    virtual void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const;

    virtual string str() const = 0;

protected:
    TypePtr type_;
};

#endif //FENNEC_VALUE_H
