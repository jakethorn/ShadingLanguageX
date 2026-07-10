//
// Created by jaket on 01/12/2025.
//

#include "runtime/ParameterList.h"

#include "runtime/Argument.h"
#include "errors/CompileError.h"
#include "runtime/utils/monomorphize.h"

namespace mxslc::runtime
{
    ParameterList::ParameterList(vector<Parameter> params) : params_{std::move(params)}
    {
        for (size_t i = 0; i < params_.size(); ++i)
            for (size_t j = i+1; j < params_.size(); ++j)
                if (params_[i].name() == params_[j].name())
                    throw CompileError{"Multiple parameters with the name '" + params_[j].name() + "'"};
    }

    ParameterList ParameterList::monomorphize(const TypePtr& template_type) const
    {
        return runtime_utils::monomorphize(params_, template_type);
    }

    bool ParameterList::contains(const string& s) const
    {
        for (const Parameter& param : params_)
        {
            if (param.name() == s)
                return true;
        }
        return false;
    }

    bool ParameterList::contains(const Argument& arg) const
    {
        if (arg.has_name())
            return contains(arg.name());
        else
            return arg.index() < params_.size();
    }

    const Parameter& ParameterList::operator[](const string& s) const
    {
        for (const Parameter& param : params_)
            if (param.name() == s)
                return param;

        throw CompileError{"Parameter not found: " + s};
    }

    const Parameter& ParameterList::operator[](const Argument& arg) const
    {
        if (arg.has_name())
            return (*this)[arg.name()];
        return (*this)[arg.index()];
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
}
