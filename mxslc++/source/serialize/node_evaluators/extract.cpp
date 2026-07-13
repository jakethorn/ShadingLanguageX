//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/extract.h"

#include "Primitive.h"
#include "runtime/interface.h"

namespace mxslc::serialize
{
    VarPtr evaluate_extract(const TypePtr&, const vector<Primitive>& values)
    {
        const Primitive& value = values[0];
        const Primitive& index = values[1];
        return create_variable(value[index]);
    }
}
