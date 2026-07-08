//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_PRIMITIVE_T_H
#define MXSLC_PRIMITIVE_T_H

#include <variant>
#include <MaterialXCore/Types.h>

#include "common.h"

namespace mxslc
{
    using primitive_t = std::variant<
        bool,
        int,
        float,
        string,
        fs::path,
        mx::Vector2,
        mx::Vector3,
        mx::Vector4,
        mx::Color3,
        mx::Color4,
        mx::Matrix33,
        mx::Matrix44
    >;
}

#endif //MXSLC_PRIMITIVE_T_H
