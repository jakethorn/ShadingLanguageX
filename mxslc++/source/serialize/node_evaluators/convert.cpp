//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/convert.h"

#include "Primitive.h"
#include "runtime/interface.h"

namespace mxslc::serialize
{
    VarPtr evaluate_convert(const TypePtr& type, const vector<Primitive>& values)
    {
        return create_variable(values[0].convert(type));
    }
}
