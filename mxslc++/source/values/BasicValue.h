//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_BASICVALUE_H
#define FENNEC_BASICVALUE_H

#include "utils/common.h"
#include "Value.h"
#include "runtime/Type.h"

class BasicValue final : public Value
{
public:
    explicit BasicValue(basic_t val) : val_{std::move(val)}, type_{val_} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        std::visit(
            [this, &node, &input_name](const auto& v) {
                node->setInputValue(input_name, v, type_.str());
            },
            val_
        );
    }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string& output_name) const override
    {
        mx::OutputPtr output = node_graph->addOutput(output_name, type_.str());
        std::visit(
            [this, &output](const auto& v) {
                output->setValue(v, type_.str());
            },
            val_
        );
    }

    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override
    {
        mx::InputPtr input = node_def->addInput(input_name, type_.str());
        std::visit(
            [this, &input](const auto& v) {
                input->setValue(v, type_.str());
            },
            val_
        );
    }

    [[nodiscard]] const Type& type() const override { return type_; }
    [[nodiscard]] string str() const override
    {
        return std::visit(
            [](const auto& v){
                std::stringstream ss;
                ss << std::boolalpha << v;
                return ss.str();
            },
            val_
        );
    }

    template<typename T>
    [[nodiscard]] bool is() const
    {
        return std::holds_alternative<T>(val_);
    }

    template<typename T>
    [[nodiscard]] T get() const
    {
        return std::get<T>(val_);
    }

private:
    basic_t val_;
    Type type_;
};

#endif //FENNEC_BASICVALUE_H
