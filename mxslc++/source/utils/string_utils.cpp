//
// Created by jaket on 31/12/2025.
//

#include "utils/string_utils.h"

#include "runtime/Type.h"

namespace mxslc::string_utils
{
    string get_postfix(const string& str, const char delim)
    {
        if (const size_t i = str.find_last_of(delim); i != string::npos)
            return str.substr(i + 1);
        return "";
    }

    bool starts_with(const string& str, const string& prefix)
    {
        return str.rfind(prefix, 0) == 0;
    }

    bool starts_with(const string& str, const char* prefix)
    {
        return str.rfind(prefix, 0) == 0;
    }

    void replace_last(string& str, const string& old_str, const string& new_str)
    {
        if (const size_t i = str.rfind(old_str); i != string::npos)
            str.replace(i, old_str.length(), new_str);
    }
}
