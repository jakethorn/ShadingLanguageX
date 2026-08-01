//
// Created by jaket on 10/07/2026.
//

#ifndef MXSLC_TYPE_UTILS_H
#define MXSLC_TYPE_UTILS_H

#include "common.h"
#include "TypeName.h"

namespace mxslc::type_utils
{
    TypePtr type_of(const vector<VarPtr>& children);

    bool contains_auto(const TypePtr& type);
    TypePtr replace_auto(const TypePtr& original_type, const TypePtr& replacement_type);

    string to_string(const vector<TypePtr>& types);

    template<typename T>
    string name_of()
    {
        return runtime_utils::TypeName::of<T>();
    }
}

#endif //MXSLC_TYPE_UTILS_H
