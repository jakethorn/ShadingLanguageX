//
// Created by jaket on 13/04/2026.
//

#ifndef MXSLC_MTLX_TYPE_OSTREAM_OPS_H
#define MXSLC_MTLX_TYPE_OSTREAM_OPS_H

#include "utils/common.h"

inline std::ostream& operator<<(std::ostream& os, const mx::Vector2& v)
{
    return os << "vec2{" << v[0] << ", " << v[1] << "}";
}

inline std::ostream& operator<<(std::ostream& os, const mx::Vector3& v)
{
    return os << "vec3{" << v[0] << ", " << v[1] << ", " << v[2] << "}";
}

inline std::ostream& operator<<(std::ostream& os, const mx::Vector4& v)
{
    return os << "vec4{" << v[0] << ", " << v[1] << ", " <<  v[2] << ", " <<  v[3] << "}";
}

inline std::ostream& operator<<(std::ostream& os, const mx::Color3& c)
{
    return os << "color3{" << c[0] << ", " << c[1] << ", " << c[2] << "}";
}

inline std::ostream& operator<<(std::ostream& os, const mx::Color4& c)
{
    return os << "Color4{" << c[0] << ", " << c[1] << ", " <<  c[2] << ", " <<  c[3] << "}";
}

inline std::ostream& operator<<(std::ostream& os, const mx::Matrix33& m)
{
    return os << "Matrix33{"
    << "{" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << "}, "
    << "{" << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << "}, "
    << "{" << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << "}}";
}

inline std::ostream& operator<<(std::ostream& os, const mx::Matrix44& m)
{
    return os << "Matrix44{"
    << "{" << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << "}, "
    << "{" << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << "}, "
    << "{" << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << "}, "
    << "{" << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << "}}";
}

#endif //MXSLC_MTLX_TYPE_OSTREAM_OPS_H