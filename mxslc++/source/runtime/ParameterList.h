//
// Created by jaket on 01/12/2025.
//

#ifndef FENNEC_PARAMETERLIST_H
#define FENNEC_PARAMETERLIST_H

#include "utils/common.h"
#include "Parameter.h"
#include "Argument.h"
#include "utils/instantiate_template_types_utils.h"

class ParameterList
{
public:
    ParameterList(vector<Parameter> params) : params_{std::move(params)} { }

    [[nodiscard]] ParameterList instantiate_template_types(const TypeInfoPtr& template_type) const
    {
        return ::instantiate_template_types(params_, template_type);
    }

    [[nodiscard]] size_t size() const { return params_.size(); }
    [[nodiscard]] bool empty() const { return params_.empty(); }

    const Parameter& operator[](const size_t i) const { return params_.at(i); }
    const Parameter& operator[](const string& s) const
    {
        for (const Parameter& param : params_)
            if (param.name() == s)
                return param;

        throw std::out_of_range{"Parameter not found:" + s};
    }
    const Parameter& operator[](const Argument& a) const;

    auto begin() { return params_.begin(); }
    auto begin() const { return params_.begin(); }

    auto end() { return params_.end(); }
    auto end() const { return params_.end(); }

private:
    vector<Parameter> params_;
};

#endif //FENNEC_PARAMETERLIST_H
