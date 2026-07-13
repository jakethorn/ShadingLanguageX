//
// Created by jaket on 14/04/2026.
//

#include "serialize/values/BasicValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize::values
{
    BasicValue::BasicValue(const mx::ValuePtr& value) : BasicValue{Primitive{value}} { }
    BasicValue::BasicValue(Primitive value) : Value{value.type()}, value_{std::move(value)} { }
    BasicValue::BasicValue(Primitive value, TypePtr type) : Value{std::move(type)}, value_{std::move(value)} { }

    bool BasicValue::equals(const ValuePtr& other) const
    {
        if (const BasicValuePtr other_basic = cast_value<BasicValue>(other))
            return (value_ == other_basic->value_).as<bool>();
        return false;
    }

    void BasicValue::set_as_node_input(const mx::InputPtr& input) const
    {
        value_.visit([this, &input](const auto& v) {
            IF_VISITED_TYPE_IS(fs::path)
                input->setValue(v.string(), type_name());
            else
                input->setValue(v, type_name());
        });
    }

    void BasicValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);

        value_.visit([this, &output](const auto& v) {
            IF_VISITED_TYPE_IS(fs::path)
                output->setValue(v.string(), type_name());
            else
                output->setValue(v, type_name());
        });
    }

    void BasicValue::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        mx::InputPtr input = node_def->addInput(input_name, type_->name());

        value_.visit([this, &input](const auto& v) {
            IF_VISITED_TYPE_IS(fs::path)
                input->setValue(v.string(), type_name());
            else
                input->setValue(v, type_name());
        });
    }

    string BasicValue::str() const
    {
        return value_.to_string();
    }

    string BasicValue::type_name() const
    {
        return type_->name();
    }
}
