//
// Created by jaket on 05/05/2026.
//

#ifndef MXSLC_ATTRIBUTELIST_H
#define MXSLC_ATTRIBUTELIST_H

#include "utils/common.h"
#include "Attribute.h"

class AttributeList
{
public:
    AttributeList() = default;
    explicit AttributeList(vector<Attribute> attrs) : attrs_{std::move(attrs)} { }

    void add_to(const mx::ElementPtr& element) const;
    void add_to(const mx::ElementPtr& element, const string& child_name) const;

private:
    vector<Attribute> attrs_;
};

#endif //MXSLC_ATTRIBUTELIST_H
