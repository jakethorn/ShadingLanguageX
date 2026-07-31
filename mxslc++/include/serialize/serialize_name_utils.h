//
// Created by jaket on 31/07/2026.
//

#ifndef MXSLC_SERIALIZE_NAME_UTILS_H
#define MXSLC_SERIALIZE_NAME_UTILS_H

#include <MaterialXCore/Node.h>

#include "common.h"

namespace mxslc::serialize
{
    inline const string RETURN_VALUE_PREFIX{"out"};
    inline const string OUT_PARAMETER_PREFIX{"outparam"};
    inline const string CLASS_FIELD_PREFIX{"this"};
    inline const string LOCAL_PREFIX{"local"};
    inline const string NONLOCAL_IN_PREFIX{"nonlocal_in"};
    inline const string NONLOCAL_OUT_PREFIX{"nonlocal_out"};
    inline const string TEMPORARY_VARIABLE_PREFIX{"var"};

    string with_prefix(const string& prefix, const string& name);
    string with_prefix(const string& prefix, const char* name);
    string with_prefix(const string& prefix, const TypePtr& type, size_t i);

    bool has_prefix(const string& str, const string& prefix);

    string remove_prefix(const string& str);

    string get_valid_node_name(const mx::GraphElementPtr& graph);

    template<typename T>
    string with_prefix(const string& prefix, const T& t)
    {
        return with_prefix(prefix, std::to_string(t));
    }
}

#endif //MXSLC_SERIALIZE_NAME_UTILS_H
