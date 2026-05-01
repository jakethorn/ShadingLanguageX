//
// Created by jaket on 11/04/2026.
//

#include "instantiate_template_types_utils.h"

#include "runtime/Type.h"

string instantiate_template_types(const string& src, const TypePtr& template_type)
{
    return src == "T" ? template_type->name() : src;
}
