//
// Created by jaket on 10/04/2026.
//

#include "evaluate_mtlx.h"

#include "values/BasicValue.h"
#include "values/Value.h"

/*


#define BINARY_OP(ltype, op, rtype) \
    if (values[0]->is<ltype>() && values[1]->is<rtype>()) \
        return std::make_shared<BasicValue>(values[0]->get<ltype>() op values[1]->get<rtype>());

namespace
{
    using BasicValues = vector<shared_ptr<BasicValue>>;

    ValuePtr evaluate_add(const BasicValues& values)
    {
        BINARY_OP(int, +, int)
        BINARY_OP(int, +, float)
        BINARY_OP(float, +, int)
        BINARY_OP(float, +, float)
        return nullptr;
    }

    unordered_map<string, std::function<ValuePtr(const BasicValues&)>> constexpr_funcs {
        {"add"s, evaluate_add}
    };

    bool is_constexpr(const FuncPtr& func, const vector<ValuePtr>& args, BasicValues& values)
    {
        if (not contains(constexpr_funcs, func->name()))
            return false;

        for (const ValuePtr& arg : args)
        {
            if (shared_ptr<BasicValue> value = std::dynamic_pointer_cast<BasicValue>(arg))
                values.push_back(value);
            else
                return false;
        }

        return true;
    }

    ValuePtr evaluate_constexpr(const FuncPtr& func, const vector<ValuePtr>& args)
    {
        if (BasicValues basic_values; is_constexpr(func, args, basic_values))
            if (ValuePtr value = constexpr_funcs.at(func->name())(basic_values))
                return value;
        return nullptr;
    }
}

#undef BINARY_OP


*/

ValuePtr evaluate_now(const string& node_name, const unordered_map<string, ValuePtr>& inputs)
{
    if (node_name != "add"s)
        return nullptr;

    for (const auto& [input_name, value] : inputs)
    {
        if (not value->is_basic())
            return nullptr;
    }

    const ValuePtr in1 = inputs.at("in1"s);
    const ValuePtr in2 = inputs.at("in2"s);

    if (in1->type() == TypeInfo::Int and in2->type() == TypeInfo::Int)
    {
        return std::make_shared<BasicValue>(in1->as_int() + in2->as_int());
    }

    if (in1->type() == TypeInfo::Int and in2->type() == TypeInfo::Float)
    {
        return std::make_shared<BasicValue>(static_cast<float>(in1->as_int()) + in2->as_float());
    }

    if (in1->type() == TypeInfo::Float and in2->type() == TypeInfo::Int)
    {
        return std::make_shared<BasicValue>(in1->as_float() + static_cast<float>(in2->as_int()));
    }

    if (in1->type() == TypeInfo::Float and in2->type() == TypeInfo::Float)
    {
        return std::make_shared<BasicValue>(in1->as_float() + in2->as_float());
    }

    return nullptr;
}
