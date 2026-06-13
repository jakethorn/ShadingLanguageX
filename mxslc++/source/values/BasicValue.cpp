//
// Created by jaket on 14/04/2026.
//

#include "BasicValue.h"

#include <sstream>

#include "CompileError.h"
#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"
#include "mtlx/mtlx_type_ostream_ops.h"

BasicValue::BasicValue(primitive_t val) : Value{Type::of(val)}, val_{std::move(val)} { }
BasicValue::BasicValue(primitive_t val, TypePtr type) : Value{std::move(type)}, val_{std::move(val)} { }

bool BasicValue::equals(const ValuePtr& other) const
{
    if (const shared_ptr<BasicValue> other_basic = std::dynamic_pointer_cast<BasicValue>(other))
        return val_ == other_basic->val_;
    return false;
}

void BasicValue::set_as_node_input(const mx::InputPtr& input) const
{
    std::visit(
        [this, &input](const auto& v) {
            using ValueType = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<ValueType, fs::path>)
            {
                input->setValue(v.string(), TypeName::Filename);
            }
            else
            {
                input->setValue(v, type_->name());
            }
        },
        val_
    );
}

void BasicValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
{
    const mx::OutputPtr output = add_or_get_output(node_graph, type_, output_name);
    std::visit(
        [this, &output](const auto& v) {
            using ValueType = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<ValueType, fs::path>)
            {
                output->setValue(v.string(), TypeName::Filename);
            }
            else
            {
                output->setValue(v, type_->name());
            }
        },
        val_
    );
}

void BasicValue::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
{
    mx::InputPtr input = node_def->addInput(input_name, type_->name());
    std::visit(
        [this, &input](const auto& v) {
            using ValueType = std::decay_t<decltype(v)>;
            if constexpr (std::is_same_v<ValueType, fs::path>)
            {
                input->setValue(v.string(), TypeName::Filename);
            }
            else
            {
                input->setValue(v, type_->name());
            }
        },
        val_
    );
}

string BasicValue::str() const
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

string BasicValue::type_name() const
{
    return type_->name();
}
