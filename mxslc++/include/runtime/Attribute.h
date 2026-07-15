//
// Created by jaket on 24/11/2025.
//

#ifndef FENNEC_ATTRIBUTE_H
#define FENNEC_ATTRIBUTE_H

#include <MaterialXCore/Element.h>

#include "common.h"
#include "utils/Stringable.h"

namespace mxslc::runtime
{
    class Attribute : public Stringable
    {
    public:
        Attribute(string name, string val) : Attribute{""s, std::move(name), std::move(val)} { }
        Attribute(string child, string name, string value) : child_{std::move(child)}, name_{std::move(name)}, value_{std::move(value)} { }

        void add_to(const mx::ElementPtr& element) const;

        string to_string() const override;

    private:
        bool has_child() const { return not child_.empty(); }

        string child_;
        string name_;
        string value_;
    };
}

#endif //FENNEC_ATTRIBUTE_H
