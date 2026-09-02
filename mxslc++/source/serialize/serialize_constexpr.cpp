//
// Created by jaket on 10/04/2026.
//

#include "serialize/serialize_constexpr.h"

#include <cassert>

#include "runtime/Function.h"
#include "runtime/interface.h"
#include "utils/primitive_utils.h"
#include "runtime/Type.h"
#include "runtime/variables/Variable.h"
#include "serialize/serialize_constexpr_wrapper.h"
#include "utils/container_utils.h"

namespace mxslc::serialize
{
    using primitive_utils::combine;
    using container_utils::contains;
    using serialize_utils::constexpr_func;
    using serialize_utils::wrap;
    using serialize_utils::wrap_creatematrix;
    using serialize_utils::wrap_switch;
    using serialize_utils::wrap_conditional_node;

    namespace
    {
        const unordered_map<string, constexpr_func> CONSTEXPR_FUNCS {
            {"add", wrap(primitive_utils::add)},
            {"subtract", wrap(primitive_utils::subtract)},
            {"multiply", wrap(primitive_utils::multiply)},
            {"divide", wrap(primitive_utils::divide)},
            {"not", wrap(primitive_utils::logical_not)},
            {"ifequal", wrap_conditional_node(primitive_utils::equal)},
            {"ifgreater", wrap_conditional_node(primitive_utils::greater)},
            {"ifgreatereq", wrap_conditional_node(primitive_utils::greater_equal)},
            {"constant", wrap(primitive_utils::constant)},
            {"combine2", wrap(primitive_utils::combine2)},
            {"combine3", wrap(primitive_utils::combine3)},
            {"combine4", wrap(primitive_utils::combine4)},
            {"separate2", wrap(primitive_utils::separate)},
            {"separate3", wrap(primitive_utils::separate)},
            {"separate4", wrap(primitive_utils::separate)},
            {"convert", wrap(primitive_utils::convert)},
            {"extract", wrap(primitive_utils::extract)},
            {"invert", wrap(primitive_utils::invert)},
            {"creatematrix", wrap_creatematrix()},
            {"invertmatrix", wrap(primitive_utils::invertmatrix)},
            {"switch", wrap_switch()},
            {"fract", wrap(primitive_utils::fract)},
            {"dotproduct", wrap(primitive_utils::dotproduct)},
        };

        bool is_constexpr(const FuncPtr& func, const ParameterValues& input_values, vector<Primitive>& comptime_values)
        {
            if (not contains(CONSTEXPR_FUNCS, func->name()))
                return false;

            for (const auto& [param, input_value] : input_values)
            {
                if (input_value->is_compile_time())
                {
                    Primitive value = input_value->compile_time_value();
                    if (value.is_null())
                    {
                        value = Primitive{
                            func->node_def()->getInput(param.name())->getValue()
                        };
                    }

                    comptime_values.push_back(std::move(value));
                }
                else
                {
                    return false;
                }
            }

            return true;
        }
    }

    VarPtr serialize_constexpr(const FuncPtr& func, const ParameterValues& input_values)
    {
        const TypePtr& return_type = func->return_type();

        if (vector<Primitive> comptime_values; is_constexpr(func, input_values, comptime_values))
        {
            if (VarPtr basic_value = CONSTEXPR_FUNCS.at(func->name())(return_type, comptime_values))
            {
                assert(basic_value->type()->equals(return_type));
                return basic_value;
            }
        }
        return nullptr;
    }
}
