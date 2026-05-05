//
// Created by jaket on 05/05/2026.
//

#include "Attribute.h"

#include "CompileError.h"

void Attribute::add_to(const mx::ElementPtr& element) const
{
    if (has_child())
    {
        if (element->getChild(child_) == nullptr)
            throw CompileError{"Attribute could not be added because there is no child element with the name '" + child_ + "'"};
        element->getChild(child_)->setAttribute(name_, value_);
    }
    else
    {
        element->setAttribute(name_, value_);
    }
}
