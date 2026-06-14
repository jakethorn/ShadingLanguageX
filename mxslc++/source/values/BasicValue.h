//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_BASICVALUE_H
#define FENNEC_BASICVALUE_H

#include "CompileError.h"
#include "primitive_t.h"
#include "Value.h"
#include "utils/common.h"
#include "utils/TypeName.h"

class BasicValue final : public Value
{
public:
    explicit BasicValue(primitive_t val);
    BasicValue(primitive_t val, TypePtr type);

    bool equals(const ValuePtr& other) const override;

    void set_as_node_input(const mx::InputPtr& input) const override;
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;
    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override;

    string str() const override;

    template<typename T>
    bool is() const
    {
        return std::holds_alternative<T>(val_);
    }

    template<typename T0, typename T1>
    bool is() const
    {
        return std::holds_alternative<T0>(val_) or std::holds_alternative<T1>(val_);
    }

    template<typename T>
    T get() const
    {
        if (is<T>())
            return std::get<T>(val_);
        throw CompileError{"Trying to access a value of type " + type_name() + " as a " + TypeName::get<T>()};
    }

private:
    string type_name() const;

    primitive_t val_;
};

#endif //FENNEC_BASICVALUE_H
