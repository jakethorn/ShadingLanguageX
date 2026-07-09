//
// Created by jaket on 14/04/2026.
//

#include "values/BasicValue.h"

#include <sstream>

#include "runtime/Type.h"
#include "mtlx/mtlx_utils.h"
#include "mtlx/mtlx_type_ostream_ops.h"
#include "values/interface.h"

namespace mxslc::values
{
    BasicValue::BasicValue(const mx::ValuePtr& value) : BasicValue{to_primitive(value)} { }
    BasicValue::BasicValue(primitive_t value) : Value{Type::of(value)}, value_{std::move(value)} { }
    BasicValue::BasicValue(primitive_t value, TypePtr type) : Value{std::move(type)}, value_{std::move(value)} { }

    bool BasicValue::equals(const ValuePtr& other) const
    {
        if (const BasicValuePtr other_basic = cast_value<BasicValue>(other))
            return value_ == other_basic->value_;
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
            value_
        );
    }

    void BasicValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        const mx::OutputPtr output = mtlx_utils::add_or_get_output(node_graph, type_, output_name);
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
            value_
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
            value_
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
            value_
        );
    }

    string BasicValue::type_name() const
    {
        return type_->name();
    }

    primitive_t BasicValue::to_primitive(const mx::ValuePtr& value)
    {
#define START_INIT primitive_t primitive_value = ""s; if constexpr (false) { }
#define INIT_PRIM(T) else if (value->isA<T>()) { primitive_value = value->asA<T>(); }

        START_INIT
        INIT_PRIM(bool)
        INIT_PRIM(int)
        INIT_PRIM(float)
        INIT_PRIM(string)
        INIT_PRIM(mx::Vector2)
        INIT_PRIM(mx::Vector3)
        INIT_PRIM(mx::Vector4)
        INIT_PRIM(mx::Color3)
        INIT_PRIM(mx::Color4)
        INIT_PRIM(mx::Matrix33)
        INIT_PRIM(mx::Matrix44)

#undef INIT_PRIM
#undef START_INIT

        return primitive_value;
    }
}
