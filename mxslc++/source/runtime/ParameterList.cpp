//
// Created by jaket on 01/12/2025.
//

#include "ParameterList.h"

#include "Argument.h"
#include "CompileError.h"

ParameterList::ParameterList(vector<Parameter> params) : params_{std::move(params)}
{
    for (size_t i = 0; i < params_.size(); ++i)
        for (size_t j = i+1; j < params_.size(); ++j)
            if (params_[i].name() == params_[j].name())
                throw CompileError{"Multiple parameters with the name '" + params_[j].name() + "'"};
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
