//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_BASICVALUE_H
#define FENNEC_BASICVALUE_H

#include "CompileError.h"
#include "Value.h"
#include "runtime/Type.h"
#include "utils/common.h"

class BasicValue final : public Value
{
public:
    explicit BasicValue(basic_t val);
    BasicValue(basic_t val, TypePtr type);

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override;
    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;
    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override;

    string str() const override;

    template<typename T>
    bool is() const
    {
        return std::holds_alternative<T>(val_);
    }

    template<typename T>
    T get() const
    {
        if (not is<T>())
            throw CompileError{"Trying to access a value of type " + type_->str() + " as a " + typeid(T).name()};
        return std::get<T>(val_);
    }

private:
    basic_t val_;
};

#endif //FENNEC_BASICVALUE_H
