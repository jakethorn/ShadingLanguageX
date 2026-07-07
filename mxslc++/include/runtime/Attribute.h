//
// Created by jaket on 24/11/2025.
//

#ifndef FENNEC_ATTRIBUTE_H
#define FENNEC_ATTRIBUTE_H

#include "utils/common.h"

class Attribute
{
public:
    Attribute(string name, string val) : Attribute{""s, std::move(name), std::move(val)} { }
    Attribute(string child, string name, string value) : child_{std::move(child)}, name_{std::move(name)}, value_{std::move(value)} { }

    void add_to(const mx::ElementPtr& element) const;

private:
    bool has_child() const { return not child_.empty(); }

    string child_;
    string name_;
    string value_;
};

#endif //FENNEC_ATTRIBUTE_H
