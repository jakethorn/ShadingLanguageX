//
// Created by jaket on 08/08/2026.
//

#include "utils/container_utils.h"

#include "runtime/Type.h"

namespace mxslc::container_utils
{
    bool contains(const string& str, const char c)
    {
        return str.find(c) != std::string::npos;
    }

    bool contains(const vector<TypePtr>& types, const TypePtr& type)
    {
        return type->is_in(types);
    }
}
