//
// Created by jaket on 12/11/2025.
//

#ifndef FENNEC_VALUE_H
#define FENNEC_VALUE_H

#include <MaterialXCore/Node.h>

#include "runtime/TypeInfo.h"
#include "utils/common.h"

class Token;

class Value
{
public:
    explicit Value(TypeInfoPtr type) : type_{std::move(type)} { }
    virtual ~Value() = default;

    [[nodiscard]] size_t subvalue_count() const { return type_->field_count(); }
    [[nodiscard]] ValuePtr subvalue(const Token& name) const;
    void set_subvalue(const Token& name, const ValuePtr& value);

    [[nodiscard]] virtual ValuePtr subvalue(size_t i) const;
    virtual void set_subvalue(size_t i, const ValuePtr& value);

    virtual void set_as_node_input(const mx::NodePtr& node, const string& input_name) const { }
    virtual void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const { }
    virtual void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const { }
    virtual void set_name(const string& name) { }

    [[nodiscard]] TypeInfoPtr type() { return type_; }

    [[nodiscard]] virtual bool is_basic() const { return false; }
    [[nodiscard]] virtual bool as_bool() const;
    [[nodiscard]] virtual int as_int() const;
    [[nodiscard]] virtual float as_float() const;
    [[nodiscard]] virtual string as_string() const;

    [[nodiscard]] virtual string str() const = 0;

protected:
    TypeInfoPtr type_;
};

#endif //FENNEC_VALUE_H
