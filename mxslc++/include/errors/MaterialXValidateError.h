//
// Created by jaket on 08/09/2026.
//

#ifndef MXSLC_MATERIALXVALIDATEERROR_H
#define MXSLC_MATERIALXVALIDATEERROR_H

#include <exception>

#include "common.h"

class MaterialXValidateError : public std::runtime_error
{
public:
    explicit MaterialXValidateError(string message) : std::runtime_error{std::move(message)} { }
};

#endif //MXSLC_MATERIALXVALIDATEERROR_H
