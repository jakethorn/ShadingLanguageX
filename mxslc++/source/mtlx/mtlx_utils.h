//
// Created by jaket on 03/01/2026.
//

#ifndef FENNEC_MTLX_UTILS_H
#define FENNEC_MTLX_UTILS_H

#include <MaterialXCore/Node.h>

#include "utils/common.h"

string as_string(const mx::NodePtr& node);
string port_name(const string& port_name, size_t i);
mx::OutputPtr add_or_get_output(const mx::NodeGraphPtr& node_graph, const string& name, const TypeInfoPtr& type);

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

#endif //FENNEC_MTLX_UTILS_H
