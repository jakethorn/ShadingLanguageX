//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_STRING_UTILS_H
#define FENNEC_STRING_UTILS_H

#include "common.h"

namespace mxslc::string_utils
{
    string get_postfix(const string& str, char delim);

    bool starts_with(const string& str, const string& prefix);
    bool starts_with(const string& str, const char* prefix);
}

#endif //FENNEC_STRING_UTILS_H
