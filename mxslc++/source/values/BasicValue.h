//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_BASICVALUE_H
#define FENNEC_BASICVALUE_H

#include "utils/common.h"
#include "Value.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/TypeInfo.h"

class BasicValue final : public Value
{
public:
    explicit BasicValue(basic_t val) : Value{std::make_shared<TypeInfo>(val)}, val_{std::move(val)} { }
    BasicValue(basic_t val, TypeInfoPtr type) : Value{std::move(type)}, val_{std::move(val)} { }

    void set_as_node_input(const mx::NodePtr& node, const string& input_name) const override
    {
        std::visit(
            [this, &node, &input_name](const auto& v) {
                node->setInputValue(input_name, v, type_->name());
            },
            val_
        );
    }

    void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override
    {
        const mx::OutputPtr output = add_or_get_output(node_graph, output_name, type_);
        std::visit(
            [this, &output](const auto& v) {
                output->setValue(v, type_->name());
            },
            val_
        );
    }

    void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override
    {
        mx::InputPtr input = node_def->addInput(input_name, type_->name());
        std::visit(
            [this, &input](const auto& v) {
                input->setValue(v, type_->name());
            },
            val_
        );
    }

    bool is_basic() const override { return true; }
    bool as_bool() const override { return get<bool>(); }
    int as_int() const override { return get<int>(); }
    float as_float() const override { return get<float>(); }
    string as_string() const override { return get<string>(); }

    string str() const override
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
    bool is() const
    {
        return std::holds_alternative<T>(val_);
    }

    template<typename T>
    T get() const
    {
        return std::get<T>(val_);
    }

private:
    basic_t val_;
};

#endif //FENNEC_BASICVALUE_H
