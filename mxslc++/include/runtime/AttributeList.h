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
        size_t size() const { return attrs_.size(); }
        void add(Attribute attr) { attrs_.push_back(std::move(attr)); }
        const vector<Attribute>& attributes() const { return attrs_; }

        string to_string() const override;

    private:
        vector<Attribute> attrs_;
    };
}

#endif //MXSLC_ATTRIBUTELIST_H
