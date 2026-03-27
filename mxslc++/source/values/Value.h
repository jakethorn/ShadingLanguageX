//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VALUE_H
#define FENNEC_VALUE_H

#include <MaterialXCore/Node.h>
#include "utils/common.h"

class Token;

class Value
{
public:
    explicit Value(TypeInfoPtr type) : type_{std::move(type)} { }
    virtual ~Value() = default;

    [[nodiscard]] virtual size_t subvalue_count() const { return 0; }
    [[nodiscard]] virtual ValuePtr subvalue(const size_t i) const;
    [[nodiscard]] virtual ValuePtr subvalue(const Token& name) const;

    virtual void set_as_node_input(const mx::NodePtr& node, const string& input_name) const { }
    virtual void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const { }
    virtual void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const { }
    virtual void set_name(const string& name) { }

    [[nodiscard]] ValuePtr cast(const TypeInfoPtr& type) const;
    [[nodiscard]] virtual ValuePtr cast_impl(const TypeInfoPtr& type) const = 0;

    [[nodiscard]] virtual TypeInfoPtr type() { return type_; }
    [[nodiscard]] virtual string str() const = 0;

protected:
    TypeInfoPtr type_;
};

#endif //FENNEC_VALUE_H
