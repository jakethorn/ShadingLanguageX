//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_ATTRIBUTELIST_H
#define MXSLC_ATTRIBUTELIST_H

#include "common.h"
#include "runtime/Attribute.h"

namespace mxslc::runtime
{
    class AttributeList : public Stringable
    {
    public:
        AttributeList() = default;
        explicit AttributeList(vector<Attribute> attrs) : attrs_{std::move(attrs)} { }

        void add_to(const mx::ElementPtr& element) const;
        void add_to(const mx::ElementPtr& element, const string& child_name) const;

        bool empty() const { return attrs_.empty(); }
        const vector<Attribute>& attributes() const { return attrs_; }

        string to_string() const override;
        string to_string(const string& delimiter) const;

    private:
        vector<Attribute> attrs_;
    };
}

#endif //MXSLC_ATTRIBUTELIST_H
