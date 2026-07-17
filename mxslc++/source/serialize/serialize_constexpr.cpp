//
// Created by jaket on 10/04/2026.
//

#include "serialize/serialize_constexpr.h"

#include <cassert>

#include "runtime/interface.h"
#include "utils/primitive_utils.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
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

    namespace
    {
        const unordered_map<string, constexpr_func> CONSTEXPR_FUNCS {
            {"add", wrap(primitive_utils::add)},
            {"subtract", wrap(primitive_utils::subtract)},
            {"multiply", wrap(primitive_utils::multiply)},
            {"divide", wrap(primitive_utils::divide)},
            {"combine2", wrap(primitive_utils::combine2)},
            {"combine3", wrap(primitive_utils::combine3)},
            {"combine4", wrap(primitive_utils::combine4)},
            {"separate2", wrap(primitive_utils::separate)},
            {"separate3", wrap(primitive_utils::separate)},
            {"separate4", wrap(primitive_utils::separate)},
            {"convert", wrap(primitive_utils::convert)},
            {"extract", wrap(primitive_utils::extract)},
            {"creatematrix", wrap_creatematrix()},
            //{"invertmatrix", wrap(primitive_utils::invertmatrix)},
            {"switch", wrap_switch()},
        };

        bool is_constexpr(const string& node_name, const ParameterValues& input_values, vector<Primitive>& basic_values)
        {
            if (not contains(CONSTEXPR_FUNCS, node_name))
                return false;

            for (const auto& [param, input_value] : input_values)
            {
                if (input_value->is_basic())
                    basic_values.push_back(input_value->basic());
                else
                    return false;
            }

            return true;
        }
    }

    VarPtr serialize_constexpr(const TypePtr& node_type, const string& node_name, const ParameterValues& input_values)
    {
        if (vector<Primitive> basic_values; is_constexpr(node_name, input_values, basic_values))
        {
            if (VarPtr basic_value = CONSTEXPR_FUNCS.at(node_name)(node_type, basic_values))
            {
                assert(basic_value->type()->equals(node_type));
                return basic_value;
            }
        }
        return nullptr;
    }
}
