//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_STRING_UTILS_H
#define FENNEC_STRING_UTILS_H

#include "common.h"

namespace mxslc::string_utils
{
    string get_postfix(const string& str, char delim);

    template<typename T>
    string str(const T& value)
    {
        return std::to_string(value);
    }
}

#endif //FENNEC_STRING_UTILS_H
