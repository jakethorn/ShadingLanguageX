//
// Created by jaket on 10/04/2026.
//

#include "serialize/serialize_constexpr.h"

#include <cassert>

#include "serialize/node_evaluators/binary_ops.h"
#include "serialize/node_evaluators/combine.h"
#include "serialize/node_evaluators/convert.h"
#include "serialize/node_evaluators/extract.h"
#include "serialize/node_evaluators/separate.h"
#include "serialize/values/Value.h"
#include "serialize/values/BasicValue.h"
#include "serialize/values/interface.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"

namespace mxslc::serialize
{
    using container_utils::contains;
    
    namespace
    {
        const unordered_map<string, std::function<VarPtr(const TypePtr& node_type, const vector<BasicValuePtr>&)>> CONSTEXPR_FUNCS {
            {"add", evaluate_add},
            {"subtract", evaluate_subtract},
            {"multiply", evaluate_multiply},
            {"divide", evaluate_divide},
            {"combine2", evaluate_combine2},
            {"combine3", evaluate_combine3},
            {"combine4", evaluate_combine4},
            {"separate2", evaluate_separate2},
            {"separate3", evaluate_separate3},
            {"separate4", evaluate_separate4},
            {"convert", evaluate_convert},
            {"extract", evaluate_extract}
        };

        bool is_constexpr(const string& node_name, const ParameterValues& input_values, vector<BasicValuePtr>& values)
        {
            if (not contains(CONSTEXPR_FUNCS, node_name))
                return false;

            for (const auto& [param, value] : input_values)
            {
                if (BasicValuePtr basic_value = cast_value<BasicValue>(value->value()))
                    values.push_back(basic_value);
                else
                    return false;
            }

            return true;
        }
    }

    VarPtr serialize_constexpr(const TypePtr& node_type, const string& node_name, const ParameterValues& input_values)
    {
        if (vector<BasicValuePtr> basic_values; is_constexpr(node_name, input_values, basic_values))
        {
            if (VarPtr value = CONSTEXPR_FUNCS.at(node_name)(node_type, basic_values))
            {
                assert(value->type()->equals(node_type));
                return value;
            }
        }
        return nullptr;
    }
}
