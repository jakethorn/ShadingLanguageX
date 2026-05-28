//
// Created by jaket on 10/04/2026.
//

#include "evaluate_mtlx.h"

#include <cassert>

#include "CompileError.h"
#include "node_evaluators/binary_ops.h"
#include "node_evaluators/combine.h"
#include "node_evaluators/convert.h"
#include "node_evaluators/separate.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "values/Value.h"
#include "values/BasicValue.h"

namespace
{
    unordered_map<string, std::function<VarPtr(const TypePtr& node_type, const vector<BasicValuePtr>&)>> constexpr_funcs {
        {"add"s, evaluate_add},
        {"subtract"s, evaluate_subtract},
        {"multiply"s, evaluate_multiply},
        {"divide"s, evaluate_divide},
        {"combine2"s, evaluate_combine2},
        {"combine3"s, evaluate_combine3},
        {"combine4"s, evaluate_combine4},
        {"separate2"s, evaluate_separate2},
        {"separate3"s, evaluate_separate3},
        {"separate4"s, evaluate_separate4},
        {"convert"s, evaluate_convert}
    };

    bool is_constexpr(const string& node_name, const ParameterValues& input_values, vector<BasicValuePtr>& values)
    {
        if (not contains(constexpr_funcs, node_name))
            return false;

        for (const auto& [param, value] : input_values)
        {
            if (BasicValuePtr basic_value = std::dynamic_pointer_cast<BasicValue>(value->value()))
                values.push_back(basic_value);
            else
                return false;
        }

        return true;
    }
}

VarPtr evaluate_now(const TypePtr& node_type, const string& node_name, const ParameterValues& input_values)
{
    if (vector<BasicValuePtr> basic_values; is_constexpr(node_name, input_values, basic_values))
    {
        if (VarPtr value = constexpr_funcs.at(node_name)(node_type, basic_values))
        {
            assert(value->type()->is_equal(node_type));
            return value;
        }
    }
    return nullptr;
}
