//
// Created by jaket on 31/12/2025.
//

#include "utils/string_utils.h"

#include "Token.h"
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

    string str(const optional<Token>& token)
    {
        return token ? token->lexeme() : "";
    }

    string str(const vector<TypePtr>& types)
    {
        if (types.empty())
            return "";

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
}
