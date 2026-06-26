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
    ParameterList() = default;
    ParameterList(vector<Parameter> params);

    ParameterList instantiate_template_types(const TypePtr& template_type) const
    {
        return ::instantiate_template_types(params_, template_type);
    }

    void init()
    {
        for (Parameter& param : params_)
            param.init();
    }

    size_t size() const { return params_.size(); }
    bool empty() const { return params_.empty(); }

    bool contains(const string& s) const;
    bool contains(const Argument& arg) const;

    const Parameter& operator[](const size_t i) const { return params_.at(i); }
    const Parameter& operator[](const string& s) const;
    const Parameter& operator[](const Argument& arg) const;

    auto begin() { return params_.begin(); }
    auto begin() const { return params_.begin(); }

    auto end() { return params_.end(); }
    auto end() const { return params_.end(); }

    string str() const;

private:
    vector<Parameter> params_;
};

#endif //FENNEC_PARAMETERLIST_H
