//
// Created by jaket on 01/12/2025.
//

#ifndef FENNEC_PARAMETERLIST_H
#define FENNEC_PARAMETERLIST_H

#include "common.h"
#include "runtime/Parameter.h"

namespace mxslc::runtime
{
    using runtime_utils::Monomorphizable;

    class Argument;

    class ParameterList : public Monomorphizable<ParameterList>, Stringable
    {
    public:
        ParameterList() = default;
        ParameterList(vector<Parameter> params);

        ParameterList monomorphize(const TypePtr& template_type) const override;

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

        string to_string() const override;

    private:
        vector<Parameter> params_;
    };
}

#endif //FENNEC_PARAMETERLIST_H
