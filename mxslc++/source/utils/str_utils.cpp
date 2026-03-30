//
// Created by jaket on 31/12/2025.
//

#include "str_utils.h"
#include "runtime/TypeInfo.h"

string get_postfix(const string& str, const char delim)
{
    if (const size_t i = str.find_last_of(delim); i != string::npos)
        return str.substr(i + 1);
    return ""s;
}

string str(const vector<TypeInfoPtr>& types)
{
    if (types.empty())
        return ""s;

    if (types.size() == 1)
        return types[0]->str();

    string result = "(";
    for (size_t i = 0; i < types.size(); ++i)
    {
        result += types[i]->str();
        if (i < types.size() - 1)
            result += ", ";
    }
    result += ")";
    return result;
}
