//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_PRIMITIVE_T_H
#define MXSLC_PRIMITIVE_T_H

#include <filesystem>
#include <string>
#include <variant>
#include <MaterialXCore/Types.h>

namespace mxslc
{
    using primitive_t = std::variant<
        bool,
        int,
        float,
        std::string,
        std::filesystem::path,
        MaterialX::Vector2,
        MaterialX::Vector3,
        MaterialX::Vector4,
        MaterialX::Color3,
        MaterialX::Color4,
        MaterialX::Matrix33,
        MaterialX::Matrix44
    >;
}

#endif //MXSLC_PRIMITIVE_T_H
