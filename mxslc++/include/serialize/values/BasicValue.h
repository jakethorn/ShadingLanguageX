//
// Created by jaket on 27/11/2025.
//

#ifndef FENNEC_BASICVALUE_H
#define FENNEC_BASICVALUE_H

#include "serialize/values/Value.h"
#include "primitive_t.h"
#include "runtime/utils/TypeName.h"
#include "errors/CompileError.h"

namespace mxslc::values
{
    using runtime_utils::TypeName;

    class BasicValue final : public Value
    {
    public:
        explicit BasicValue(const mx::ValuePtr& value);
        explicit BasicValue(primitive_t value);
        BasicValue(primitive_t value, TypePtr type);

        bool equals(const ValuePtr& other) const override;

        void set_as_node_input(const mx::InputPtr& input) const override;
        void set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const override;
        void set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const override;

        string str() const override;

        template<typename T>
        bool is() const
        {
            return std::holds_alternative<T>(value_);
        }

        template<typename T0, typename T1>
        bool is() const
        {
            return std::holds_alternative<T0>(value_) or std::holds_alternative<T1>(value_);
        }

        template<typename T>
        T get() const
        {
            if (is<T>())
                return std::get<T>(value_);
            throw CompileError{"Trying to access a value of type " + type_name() + " as a " + TypeName::of<T>()};
        }

    private:
        string type_name() const;

        static primitive_t to_primitive(const mx::ValuePtr& value);

        primitive_t value_;
    };
}

#endif //FENNEC_BASICVALUE_H
