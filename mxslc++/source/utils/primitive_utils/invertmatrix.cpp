//
// Created by jaket on 16/07/2026.
//

#include "utils/primitive_utils.h"

namespace mxslc::primitive_utils
{
    namespace
    {
        mx::Matrix33 invertmatrix33(const mx::Matrix33& m)
        {
            const float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2];
            const float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2];
            const float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2];

            const float inv00 = m11 * m22 - m12 * m21;
            const float inv01 = m02 * m21 - m01 * m22;
            const float inv02 = m01 * m12 - m02 * m11;
            const float inv10 = m12 * m20 - m10 * m22;
            const float inv11 = m00 * m22 - m02 * m20;
            const float inv12 = m02 * m10 - m00 * m12;
            const float inv20 = m10 * m21 - m11 * m20;
            const float inv21 = m01 * m20 - m00 * m21;
            const float inv22 = m00 * m11 - m01 * m10;

            const float det = m00 * inv00 + m01 * inv10 + m02 * inv20;

            if (det == 0)
                return mx::Matrix33{};

            const float inv_det = 1 / det;

            return mx::Matrix33 {
                inv00 * inv_det,
                inv01 * inv_det,
                inv02 * inv_det,
                inv10 * inv_det,
                inv11 * inv_det,
                inv12 * inv_det,
                inv20 * inv_det,
                inv21 * inv_det,
                inv22 * inv_det
            };
        }

        mx::Matrix44 invertmatrix44(const mx::Matrix44& m)
        {
            const float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
            const float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
            const float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
            const float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];

            const float inv00 =  m11 * m22 * m33 - m11 * m23 * m32 - m21 * m12 * m33 + m21 * m13 * m32 + m31 * m12 * m23 - m31 * m13 * m22;
            const float inv10 = -m10 * m22 * m33 + m10 * m23 * m32 + m20 * m12 * m33 - m20 * m13 * m32 - m30 * m12 * m23 + m30 * m13 * m22;
            const float inv20 =  m10 * m21 * m33 - m10 * m23 * m31 - m20 * m11 * m33 + m20 * m13 * m31 + m30 * m11 * m23 - m30 * m13 * m21;
            const float inv30 = -m10 * m21 * m32 + m10 * m22 * m31 + m20 * m11 * m32 - m20 * m12 * m31 - m30 * m11 * m22 + m30 * m12 * m21;
            const float inv01 = -m01 * m22 * m33 + m01 * m23 * m32 + m21 * m02 * m33 - m21 * m03 * m32 - m31 * m02 * m23 + m31 * m03 * m22;
            const float inv11 =  m00 * m22 * m33 - m00 * m23 * m32 - m20 * m02 * m33 + m20 * m03 * m32 + m30 * m02 * m23 - m30 * m03 * m22;
            const float inv21 = -m00 * m21 * m33 + m00 * m23 * m31 + m20 * m01 * m33 - m20 * m03 * m31 - m30 * m01 * m23 + m30 * m03 * m21;
            const float inv31 =  m00 * m21 * m32 - m00 * m22 * m31 - m20 * m01 * m32 + m20 * m02 * m31 + m30 * m01 * m22 - m30 * m02 * m21;
            const float inv02 =  m01 * m12 * m33 - m01 * m13 * m32 - m11 * m02 * m33 + m11 * m03 * m32 + m31 * m02 * m13 - m31 * m03 * m12;
            const float inv12 = -m00 * m12 * m33 + m00 * m13 * m32 + m10 * m02 * m33 - m10 * m03 * m32 - m30 * m02 * m13 + m30 * m03 * m12;
            const float inv22 =  m00 * m11 * m33 - m00 * m13 * m31 - m10 * m01 * m33 + m10 * m03 * m31 + m30 * m01 * m13 - m30 * m03 * m11;
            const float inv32 = -m00 * m11 * m32 + m00 * m12 * m31 + m10 * m01 * m32 - m10 * m02 * m31 - m30 * m01 * m12 + m30 * m02 * m11;
            const float inv03 = -m01 * m12 * m23 + m01 * m13 * m22 + m11 * m02 * m23 - m11 * m03 * m22 - m21 * m02 * m13 + m21 * m03 * m12;
            const float inv13 =  m00 * m12 * m23 - m00 * m13 * m22 - m10 * m02 * m23 + m10 * m03 * m22 + m20 * m02 * m13 - m20 * m03 * m12;
            const float inv23 = -m00 * m11 * m23 + m00 * m13 * m21 + m10 * m01 * m23 - m10 * m03 * m21 - m20 * m01 * m13 + m20 * m03 * m11;
            const float inv33 =  m00 * m11 * m22 - m00 * m12 * m21 - m10 * m01 * m22 + m10 * m02 * m21 + m20 * m01 * m12 - m20 * m02 * m11;

            const float det = m00 * inv00 + m01 * inv10 + m02 * inv20 + m03 * inv30;

            if (det == 0)
                return mx::Matrix44{};

            const float inv_det = 1 / det;

            return mx::Matrix44 {
                inv00 * inv_det,
                inv01 * inv_det,
                inv02 * inv_det,
                inv03 * inv_det,
                inv10 * inv_det,
                inv11 * inv_det,
                inv12 * inv_det,
                inv13 * inv_det,
                inv20 * inv_det,
                inv21 * inv_det,
                inv22 * inv_det,
                inv23 * inv_det,
                inv30 * inv_det,
                inv31 * inv_det,
                inv32 * inv_det,
                inv33 * inv_det
            };
        }
    }

    Primitive invertmatrix(const Primitive& in)
    {
        if (in.is_a<mx::Matrix33>())
            return invertmatrix33(in.as<mx::Matrix33>());
        else
            return invertmatrix44(in.as<mx::Matrix44>());
    }
}