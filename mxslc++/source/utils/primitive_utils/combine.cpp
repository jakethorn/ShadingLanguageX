//
// Created by jaket on 12/07/2026.
//

#include "utils/primitive_utils.h"
#include "runtime/Type.h"

namespace mxslc::primitive_utils
{
    Primitive combine2(const Primitive& in1, const Primitive& in2, const TypePtr& type)
    {
        return combine({in1, in2}, type);
    }

    Primitive combine3(const Primitive& in1, const Primitive& in2, const Primitive& in3, const TypePtr& type)
    {
        return combine({in1, in2, in3}, type);
    }

    Primitive combine4(const Primitive& in1, const Primitive& in2, const Primitive& in3, const Primitive& in4, const TypePtr& type)
    {
        return combine({in1, in2, in3, in4}, type);
    }

    Primitive combine(const vector<Primitive>& in, const TypePtr& type)
    {
        vector<float> channels;
        for (const Primitive& p : in)
        {
            if (p.is_castable_to<float>())
            {
                channels.push_back(p.cast<float>());
            }
            else if (p.is_vector_type())
            {
                for (const Primitive& q : separate(p))
                    channels.push_back(q.as<float>());
            }
            else
            {
                throw CompileError{"Cannot combine a value of type '" + p.type_name() + "'"};
            }
        }

        if (channels.size() == 1 and type->is<float>())
            return channels[0];

#define COMBINE_IF(t) if (channels.size() == t::numElements() and type->is<t>()) { return t{channels}; }
        COMBINE_IF(mx::Vector2)
        COMBINE_IF(mx::Vector3)
        COMBINE_IF(mx::Vector4)
        COMBINE_IF(mx::Color3)
        COMBINE_IF(mx::Color4)
#undef COMBINE_IF

        throw CompileError{"Number of channels (" + std::to_string(channels.size()) + ") cannot combine to a value of type '" + type->name() + "'"};
    }
}
