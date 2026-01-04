//
// Created by jaket on 31/12/2025.
//

#include "str_utils.h"

string get_postfix(const string& str, const char delim)
{
    if (const size_t i = str.find_last_of(delim); i != string::npos)
        return str.substr(i + 1);
    return ""s;
}
