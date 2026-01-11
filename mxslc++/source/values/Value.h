//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VALUE_H
#define FENNEC_VALUE_H

#include <MaterialXCore/Node.h>
#include "utils/common.h"

class Type;

class Value
{
public:
    Value() = default;
    virtual ~Value() = default;

    [[nodiscard]] virtual size_t subvalue_count() const { return 0; }
    [[nodiscard]] virtual ValuePtr subvalue(const size_t i) const { return nullptr; }

    virtual void set_as_node_input(const mx::NodePtr& node, const string& input_name) const { }
    virtual void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const { }
    virtual void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const { }

    [[nodiscard]] virtual const Type& type() const = 0;
    [[nodiscard]] virtual string str() const = 0;
};

#endif //FENNEC_VALUE_H
