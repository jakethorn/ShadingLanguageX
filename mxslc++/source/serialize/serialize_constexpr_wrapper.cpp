//
// Created by jaket on 16/07/2026.
//

#include "serialize/serialize_constexpr_wrapper.h"

#include "utils/primitive_utils.h"

namespace mxslc::serialize_utils
{
    constexpr_func wrap_creatematrix()
    {
        return [](const TypePtr& type, const vector<Primitive>& values) -> VarPtr
        {
            if (values.size() == 3)
            {
                return create_variable(primitive_utils::creatematrix(values[0], values[1], values[2], type));
            }
            else
            {
                return create_variable(primitive_utils::creatematrix(values[0], values[1], values[2], values[3]));
            }
        };
    }

    constexpr_func wrap_conditional_node(const std::function<Primitive(const Primitive&, const Primitive&)>& pred)
    {
        return [pred](const TypePtr& type, const vector<Primitive>& values) -> VarPtr
        {
            return create_variable(
                values.size() == 2 ?
                pred(values[0], values[1]) :
                pred(values[0], values[1]) ? values[2] : values[3]
            );
        };
    }

    constexpr_func wrap_switch()
    {
        return [](const TypePtr& type, const vector<Primitive>& values) -> VarPtr
        {
            Primitive which = values.back();
            vector<Primitive> ins = values;
            ins.pop_back();
            return create_variable(primitive_utils::switch_(ins, which, type));
        };
    }
}
