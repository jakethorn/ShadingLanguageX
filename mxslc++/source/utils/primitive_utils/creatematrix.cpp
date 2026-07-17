//
// Created by jaket on 16/07/2026.
//

#include "utils/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    namespace
    {
        void extend(vector<float>& values, const Primitive& in, const std::initializer_list<float>& default_values, const TypePtr& type)
        {
            if (in.is_a<mx::Vector3>())
                values.insert(values.end(), {in[0].as<float>(), in[1].as<float>(), in[2].as<float>()});
            else
                values.insert(values.end(), default_values);

            if (type and type->is<mx::Matrix44>())
                values.push_back(0.0f);
        }
    }

    Primitive creatematrix(const Primitive& in1, const Primitive& in2, const Primitive& in3, const TypePtr& type)
    {
        vector<float> values;
        extend(values, in1, {1.0f, 0.0f, 0.0f}, type);
        extend(values, in2, {0.0f, 1.0f, 0.0f}, type);
        extend(values, in3, {0.0f, 0.0f, 1.0f}, type);

        if (type and type->is<mx::Matrix44>())
        {
            values.insert(values.end(), {0.0f, 0.0f, 0.0f, 1.0f});
            return mx::Matrix44(values.data(), values.data() + values.size());
        }

        return mx::Matrix33(values.data(), values.data() + values.size());
    }

    namespace
    {
        void extend(vector<float>& values, const Primitive& in, const std::initializer_list<float>& default_values)
        {
            if (in.is_a<mx::Vector4>())
                values.insert(values.end(), {in[0].as<float>(), in[1].as<float>(), in[2].as<float>(), in[3].as<float>()});
            else
                values.insert(values.end(), default_values);
        }
    }

    Primitive creatematrix(const Primitive& in1, const Primitive& in2, const Primitive& in3, const Primitive& in4)
    {
        vector<float> values;
        extend(values, in1, {1.0f, 0.0f, 0.0f, 0.0f});
        extend(values, in2, {0.0f, 1.0f, 0.0f, 0.0f});
        extend(values, in3, {0.0f, 0.0f, 1.0f, 0.0f});
        extend(values, in4, {0.0f, 0.0f, 0.0f, 1.0f});
        return mx::Matrix44(values.data(), values.data() + values.size());
    }
}
