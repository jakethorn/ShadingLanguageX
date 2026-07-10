//
// Created by jaket on 11/04/2026.
//

#include "runtime/utils/monomorphize.h"

#include "runtime/Type.h"

namespace mxslc::runtime_utils
{
    string monomorphize(const string& src, const TypePtr& template_type)
    {
        return src == "T" ? template_type->name() : src;
    }
}
