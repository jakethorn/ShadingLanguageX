//
// Created by jaket on 01/12/2025.
//

#include "ParameterList.h"

#include "Argument.h"

ParameterList::ParameterList(vector<Parameter> params) : params_{std::move(params)}
{
    vector<string> names;
    for (const Parameter& param : params_)
    {
        if (contains(names, param.name()))
            throw CompileError{param.name_token(), "Multiple parameters with the name '" + param.name() + "'"};
        names.push_back(param.name());
    }
}

const Parameter& ParameterList::operator[](const Argument& a) const
{
    if (a.has_name())
        return (*this)[a.name()];
    return (*this)[a.index()];
}

string ParameterList::str() const
{
    string result;
    for (const Parameter& param : *this)
        result += param.str() + ", ";
    if (result.size() >= 2)
        result.resize(result.size() - 2);
    return result;
}
