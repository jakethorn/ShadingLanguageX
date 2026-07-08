//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_MTLX_TYPE_BINARY_OPS_H
#define MXSLC_MTLX_TYPE_BINARY_OPS_H

namespace mxslc
{
    template<typename T>
    T operator+(const T& t, const float f)
    {
        return t + T{f};
    }

    template<typename T>
    T operator-(const T& t, const float f)
    {
        return t - T{f};
    }

    template<typename T>
    T operator*(const T& t, const float f)
    {
        return t * T{f};
    }

    template<typename T>
    T operator/(const T& t, const float f)
    {
        return t / T{f};
    }
}

#endif //MXSLC_MTLX_TYPE_BINARY_OPS_H
