//
// Created by jaket on 03/01/2026.
//

#include "ArgumentList.h"
#include "Parameter.h"
#include "ParameterList.h"
#include "utils/instantiate_template_types_utils.h"

ArgumentList ArgumentList::instantiate_template_types(const TypeInfoPtr& template_type) const
{
    return ::instantiate_template_types(args_, template_type);
}

vector<VarPtr2> ArgumentList::evaluate() const
{
    vector<VarPtr2> values;
    values.reserve(args_.size());
    for (const Argument& arg : args_)
        values.push_back(arg.evaluate());
    return values;
}

VarPtr2 ArgumentList::evaluate(const Parameter& param) const
{
    VarPtr2 value;
    if (const Argument* arg = (*this)[param])
        value = arg->evaluate();
    else if (param.has_default_value())
        value = param.evaluate();
    else
        throw CompileError{"Function call missing argument for parameter '" + param.name() + "'"};
    return value;
}

vector<pair<const Parameter&, VarPtr2>> ArgumentList::evaluate(const ParameterList& params) const
{
    vector<pair<const Parameter&, VarPtr2>> result;
    result.reserve(params.size());
    for (const Parameter& param : params)
        result.emplace_back(param, evaluate(param));
    return result;
}

const Argument* ArgumentList::operator[](const Parameter& param) const
{
    for (const Argument& arg : args_)
    {
        if (arg.has_name())
        {
            if (arg.name() == param.name())
                return &arg;
        }
        else
        {
            if (arg.index() == param.index())
                return &arg;
        }
    }
    return nullptr;
}
