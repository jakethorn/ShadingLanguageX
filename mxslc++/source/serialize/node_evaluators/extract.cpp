//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/extract.h"

#include "serialize/values/BasicValue.h"
#include "runtime/interface.h"

#define EXTRACT(fromtype, totype) \
    if (in->is<fromtype>()) \
        return create_variable(totype{in->get<fromtype>()[index->get<int>()]});

namespace mxslc::serialize
{
    VarPtr evaluate_extract(const TypePtr&, const vector<BasicValuePtr>& values)
    {
        const BasicValuePtr& in = values[0];
        const BasicValuePtr& index = values[1];

        EXTRACT(mx::Vector2, float)
        EXTRACT(mx::Vector3, float)
        EXTRACT(mx::Vector4, float)
        EXTRACT(mx::Color3, float)
        EXTRACT(mx::Color4, float)
        EXTRACT(mx::Matrix33, mx::Vector3)
        EXTRACT(mx::Matrix44, mx::Vector4)

        return nullptr;
    }
}

#undef EXTRACT
