//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_ARGUMENTLIST_H
#define FENNEC_ARGUMENTLIST_H

#include "utils/common.h"
#include "Argument.h"

class Parameter;
class ParameterList;

class ArgumentList
{
public:
    ArgumentList(vector<Argument> args) : args_{std::move(args)} { }

    template <typename... Exprs>
    explicit ArgumentList(Exprs&&... exprs)
    {
        args_.reserve(sizeof...(exprs));

        size_t i = 0;
        (args_.emplace_back(std::forward<Exprs>(exprs), i++), ...);
    }

    ArgumentList instantiate_template_types(const TypeInfoPtr& template_type) const;

    VarPtr evaluate(const Parameter& param) const;
    vector<pair<const Parameter&, VarPtr>> evaluate(const ParameterList& params) const;

    size_t size() const { return args_.size(); }
    bool empty() const { return args_.empty(); }

    const Argument& operator[](const size_t i) const { return args_.at(i); }
    const Argument* operator[](const Parameter& param) const;

    auto begin() { return args_.begin(); }
    auto begin() const { return args_.begin(); }

    auto end() { return args_.end(); }
    auto end() const { return args_.end(); }

private:
    vector<Argument> args_;
};

#endif //FENNEC_ARGUMENTLIST_H
