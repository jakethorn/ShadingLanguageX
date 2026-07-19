//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_ARGUMENTLIST_H
#define FENNEC_ARGUMENTLIST_H

#include "common.h"
#include "runtime/Argument.h"

namespace mxslc::runtime
{
    using runtime_utils::Monomorphizable;

    class Parameter;
    class ParameterList;

    class ArgumentList : public Monomorphizable<ArgumentList>, public Stringable
    {
    public:
        ArgumentList() = default;
        ArgumentList(vector<Argument> args);
        ArgumentList(const vector<ExprPtr>& exprs);
        ArgumentList(const vector<VarPtr>& values);
        ArgumentList(const vector<Primitive>& values);
        ArgumentList(const VarPtr& value);

        template <typename... Exprs>
        explicit ArgumentList(Exprs&&... exprs)
        {
            args_.reserve(sizeof...(exprs));

            size_t i = 0;
            (args_.emplace_back(std::forward<Exprs>(exprs), i++), ...);
        }

        template<typename T>
        void append(T expr)
        {
            args_.emplace_back(T, size());
        }

        ArgumentList monomorphize(const TypePtr& template_type) const override;

        VarPtr evaluate(const Parameter& param) const;
        ParameterValues evaluate(const ParameterList& params) const;

        size_t size() const { return args_.size(); }
        bool empty() const { return args_.empty(); }

        const Argument& operator[](const size_t i) const { return args_.at(i); }
        const Argument* operator[](const Parameter& param) const;

        auto begin() { return args_.begin(); }
        auto begin() const { return args_.begin(); }

        auto end() { return args_.end(); }
        auto end() const { return args_.end(); }

        string to_string() const override;

    private:
        vector<Argument> args_;
    };
}

#endif //FENNEC_ARGUMENTLIST_H
