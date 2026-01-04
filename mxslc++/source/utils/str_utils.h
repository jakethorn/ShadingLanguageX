//
// Created by jaket on 31/12/2025.
//

#ifndef FENNEC_STR_UTILS_H
#define FENNEC_STR_UTILS_H

#include "common.h"

string get_postfix(const string& str, char delim);

template<typename T>
string str(const T& t)
{
    return std::to_string(t);
}

#endif //FENNEC_STR_UTILS_H
