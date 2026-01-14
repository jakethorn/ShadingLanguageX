//
// Created by jaket on 11/01/2026.
//

#ifndef MXSLC_UNNAMEDSTRUCTVALUE_H
#define MXSLC_UNNAMEDSTRUCTVALUE_H

#include "Value.h"
#include "mtlx/mtlx_utils.h"
#include "runtime/Type.h"
#include "utils/str_utils.h"

class UnnamedStructValue final : public Value
{
public:
    explicit UnnamedStructValue(vector<ValuePtr> values) : values_{std::move(values)}
    {
        vector<Type> types;
        types.reserve(values_.size());
        for (const ValuePtr& val : values_)
            types.push_back(val->type());

        type_ = Type{std::move(types)};
    }

    [[nodiscard]] size_t subvalue_count() const override { return values_.size(); }
    [[nodiscard]] ValuePtr subvalue(const size_t i) const override { return values_.at(i); }

    void set_as_node_graph_output(const mx::GraphElementPtr& node_graph, const string&) const override
    {
        for (size_t i = 0; i < values_.size(); ++i)
            values_[i]->set_as_node_graph_output(node_graph, get_output_name(i));
    }

    const Type& type() const override { return type_; }
    string str() const override
    {
        string result;
        for (size_t i = 0; i < values_.size(); ++i)
        {
            result += "index " + ::str(i) + ": " + values_[i]->str();
            if (i < values_.size() - 1)
                result += "\n";
        }

        return result;
    }

private:
    vector<ValuePtr> values_;
    Type type_;
};

#endif //MXSLC_UNNAMEDSTRUCTVALUE_H
