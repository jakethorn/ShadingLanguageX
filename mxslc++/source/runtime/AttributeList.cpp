//
// Created by jaket on 05/05/2026.
//

#include "runtime/AttributeList.h"

namespace mxslc::runtime
{
    void AttributeList::add_to(const mx::ElementPtr& element) const
    {
        for (const Attribute& attr : attrs_)
            attr.add_to(element);
    }

    void AttributeList::add_to(const mx::ElementPtr& element, const string& child_name) const
    {
        if (const mx::ElementPtr child = element->getChild(child_name))
            add_to(child);
    }
}
