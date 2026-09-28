//
// Created by jaket on 31/12/2025.
//

#include "utils/string_utils.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>

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

    string float_to_string(const float value)
    {
        // shortest representation that reads back as the same float, always recognisable as a float literal
        const float magnitude = std::fabs(value);
        const bool use_fixed = magnitude == 0.0f or (magnitude >= 1e-5f and magnitude < 1e16f);

        char buffer[64];
        for (int precision = 1; precision <= 40; ++precision)
        {
            if (use_fixed)
                std::snprintf(buffer, sizeof(buffer), "%.*f", precision, value);
            else
                std::snprintf(buffer, sizeof(buffer), "%.*e", precision - 1, value);

            if (std::strtof(buffer, nullptr) == value)
                break;
        }

        string result{buffer};
        if (not use_fixed and result.find('.') == string::npos)
            result.insert(result.find('e'), ".0");
        return result;
    }

    string indent(const string& str, const string& indentation)
    {
        string result;
        size_t start = 0;
        while (start <= str.size())
        {
            size_t end = str.find('\n', start);
            if (end == string::npos)
                end = str.size();

            const string line = str.substr(start, end - start);
            if (not line.empty())
                result += indentation + line;
            if (end < str.size())
                result += '\n';

            start = end + 1;
        }
        return result;
    }
}
