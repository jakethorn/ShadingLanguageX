//
// Created by jaket on 11/07/2026.
//

#ifndef MXSLC_PRIMITIVE_UTILS_H
#define MXSLC_PRIMITIVE_UTILS_H

#include "Primitive.h"

namespace mxslc::primitive_utils
{
    // binary arithmatic operators
    Primitive add(const Primitive& in1, const Primitive& in2);
    Primitive subtract(const Primitive& in1, const Primitive& in2);
    Primitive multiply(const Primitive& in1, const Primitive& in2);
    Primitive divide(const Primitive& in1, const Primitive& in2);

    // binary logical operators
    Primitive logical_and(const Primitive& in1, const Primitive& in2);
    Primitive logical_or(const Primitive& in1, const Primitive& in2);
    Primitive logical_xor(const Primitive& in1, const Primitive& in2);

    // binary comparison operators
    Primitive equal(const Primitive& in1, const Primitive& in2);
    Primitive not_equal(const Primitive& in1, const Primitive& in2);
    Primitive greater(const Primitive& in1, const Primitive& in2);
    Primitive less(const Primitive& in1, const Primitive& in2);
    Primitive greater_equal(const Primitive& in1, const Primitive& in2);
    Primitive less_equal(const Primitive& in1, const Primitive& in2);

    // unary operators
    Primitive positive(const Primitive& in);
    Primitive negative(const Primitive& in);
    Primitive logical_not(const Primitive& in);

    Primitive combine2(const Primitive& in1, const Primitive& in2, const TypePtr& type);
    Primitive combine3(const Primitive& in1, const Primitive& in2, const Primitive& in3, const TypePtr& type);
    Primitive combine4(const Primitive& in1, const Primitive& in2, const Primitive& in3, const Primitive& in4, const TypePtr& type);
    Primitive combine(const vector<Primitive>& in, const TypePtr& type);
    Primitive convert(const Primitive& in, const TypePtr& type);
    Primitive extract(const Primitive& in, size_t index);
    Primitive extract(const Primitive& in, const Primitive& index);
    std::array<Primitive, 2> separate2(const Primitive& in);
    std::array<Primitive, 3> separate3(const Primitive& in);
    std::array<Primitive, 4> separate4(const Primitive& in);
    vector<Primitive> separate(const Primitive& in);
}

#endif //MXSLC_PRIMITIVE_UTILS_H
