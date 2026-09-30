//
// Created by jaket on 31/07/2026.
//

#include "serialize/name_prefix_utils.h"

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

    bool has_prefix(const mx::ElementPtr& elem, const string& prefix)
    {
        return has_prefix(elem->getName(), prefix);
    }

    string remove_prefix(const string& str)
    {
        const size_t pos = str.find("__");
        if (pos != string::npos)
            return str.substr(pos + 2);
        return str;
    }

    string remove_prefix(const string& str, const string& prefix)
    {
        if (string_utils::starts_with(str, prefix + "__"))
            return remove_prefix(str);
        return str;
    }

    string without_prefix(const mx::ElementPtr& elem)
    {
        return remove_prefix(elem->getName());
    }

    string without_prefix(const mx::ElementPtr& elem, const string& prefix)
    {
        return remove_prefix(elem->getName(), prefix);
    }

    string get_valid_node_name(const mx::GraphElementPtr& graph)
    {
        return graph->createValidChildName(with_prefix(TEMPORARY_VARIABLE_PREFIX, "0"));
    }
}
