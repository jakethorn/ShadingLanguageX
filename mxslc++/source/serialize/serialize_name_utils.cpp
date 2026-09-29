//
// Created by jaket on 31/07/2026.
//

#include "serialize/serialize_name_utils.h"

#include <algorithm>

#include "runtime/Type.h"
#include "utils/string_utils.h"

namespace mxslc::serialize
{
    string with_prefix(const string& prefix, const string& name)
    {
        return prefix + "__" + name;
    }

    string with_prefix(const string& prefix, const char* name)
    {
        return with_prefix(prefix, string{name});
    }

    string with_prefix(const string& prefix, const TypePtr& type, const size_t i)
    {
        const string& field_name = type->field_name(i);
        const string name = field_name.empty() ? std::to_string(i) : field_name;
        return with_prefix(prefix, name);
    }

    bool has_prefix(const string& str, const string& prefix)
    {
        return string_utils::starts_with(str, prefix + "__");
    }

    string remove_prefix(const string& str)
    {
        const size_t pos = str.find("__");
        if (pos != string::npos)
            return str.substr(pos + 2);
        return str;
    }

    string get_valid_node_name(const mx::GraphElementPtr& graph)
    {
        return graph->createValidChildName(with_prefix(TEMPORARY_VARIABLE_PREFIX, "0"));
    }

    bool is_temporary_node_name(const string& name)
    {
        if (not has_prefix(name, TEMPORARY_VARIABLE_PREFIX))
            return false;
        const string number = remove_prefix(name);
        return not number.empty() and std::all_of(number.begin(), number.end(), [](const char c) { return std::isdigit(static_cast<unsigned char>(c)); });
    }

    string assigned_node_name(const string& variable_name)
    {
        return with_prefix(TEMPORARY_VARIABLE_PREFIX, variable_name + "__1");
    }
}
