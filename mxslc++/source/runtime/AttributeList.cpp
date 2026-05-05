//
// Created by jaket on 05/05/2026.
//

#include "AttributeList.h"

void AttributeList::add_to(const mx::ElementPtr& element) const
{
    for (const Attribute& attr : attrs_)
        attr.add_to(element);
}

void AttributeList::add_to(const mx::ElementPtr& element, const string& child_name) const
{
    add_to(element->getChild(child_name));
}
