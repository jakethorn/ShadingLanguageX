//
// Created by jaket on 14/04/2026.
//

#include "serialize/values/CompileTimeValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize::values
{
    CompileTimeValue::CompileTimeValue(const mx::ValuePtr& value) : CompileTimeValue{Primitive{value}} { }
    CompileTimeValue::CompileTimeValue(Primitive value) : Value{value.type()}, value_{std::move(value)} { }
    CompileTimeValue::CompileTimeValue(Primitive value, TypePtr type) : Value{std::move(type)}, value_{std::move(value)} { }

    bool CompileTimeValue::equals(const ValuePtr& other) const
    {
        if (const CompileTimeValuePtr other_basic = cast_value<CompileTimeValue>(other))
            return (value_ == other_basic->value_).as<bool>();
        return false;
    }

    void CompileTimeValue::set_as_node_input(const mx::InputPtr& input) const
    {
        value_.visit([this, &input](const auto& v) {
            IF_VISITED_TYPE_IS(std::monostate)
                mtlx_utils::remove_port(input);
            else IF_VISITED_TYPE_IS(fs::path)
                input->setValue(v.string(), type_name());
            else
                input->setValue(v, type_name());
        });
    }

    void CompileTimeValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);

        value_.visit([this, &output](const auto& v) {
            IF_VISITED_TYPE_IS(std::monostate)
                mtlx_utils::remove_port(output);
            else IF_VISITED_TYPE_IS(fs::path)
                output->setValue(v.string(), type_name());
            else
                output->setValue(v, type_name());
        });
    }

    void CompileTimeValue::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        mx::InputPtr input = node_def->addInput(input_name, type_->name());

        value_.visit([this, &input](const auto& v) {
            IF_VISITED_TYPE_IS(std::monostate)
                mtlx_utils::remove_port(input);
            else IF_VISITED_TYPE_IS(fs::path)
                input->setValue(v.string(), type_name());
            else
                input->setValue(v, type_name());
        });
    }

    string CompileTimeValue::to_string() const
    {
        return value_.to_string();
    }

    string CompileTimeValue::type_name() const
    {
        return type_->name();
    }
}
