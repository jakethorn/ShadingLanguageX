//
// Created by jaket on 10/04/2026.
//

#include "evaluate_mtlx.h"

#include "statements/Statement.h"
#include "runtime/Function.h"
#include "runtime/Type.h"
#include "runtime/Variable.h"
#include "values/Value.h"
#include "values/BasicValue.h"

#define BINARY_OP(ltype, op, rtype) \
    if (values[0]->is<ltype>() && values[1]->is<rtype>()) \
        return Variable::create(std::make_shared<BasicValue>(values[0]->get<ltype>() op values[1]->get<rtype>()));

namespace
{
    using BasicValues = vector<shared_ptr<BasicValue>>;

    VarPtr evaluate_add(const BasicValues& values)
    {
        BINARY_OP(int, +, int)
        BINARY_OP(int, +, float)
        BINARY_OP(float, +, int)
        BINARY_OP(float, +, float)
        return nullptr;
    }

    unordered_map<string, std::function<VarPtr(const BasicValues&)>> constexpr_funcs {
        {"add"s, evaluate_add}
    };

    bool is_constexpr(const string& node_name, const vector<pair<const Parameter&, VarPtr>>& args, BasicValues& values)
    {
        if (not contains(constexpr_funcs, node_name))
            return false;

        for (const auto& [param, arg] : args)
        {
            if (shared_ptr<BasicValue> value = std::dynamic_pointer_cast<BasicValue>(arg->value()))
                values.push_back(value);
            else
                return false;
        }

        return true;
    }
}

VarPtr evaluate_now(const string& node_name, const vector<pair<const Parameter&, VarPtr>>& arg_values)
{
    if (BasicValues basic_values; is_constexpr(node_name, arg_values, basic_values))
        if (VarPtr value = constexpr_funcs.at(node_name)(basic_values))
            return value;
    return nullptr;
}

#undef BINARY_OP
