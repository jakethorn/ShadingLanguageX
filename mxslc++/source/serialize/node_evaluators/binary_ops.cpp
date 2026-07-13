//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/binary_ops.h"

#include "Primitive.h"
#include "runtime/interface.h"

namespace mxslc::serialize
{
    VarPtr evaluate_add(const TypePtr&, const vector<Primitive>& values)
    {
        return create_variable(values[0] + values[1]);
    }

    VarPtr evaluate_subtract(const TypePtr&, const vector<Primitive>& values)
    {
        return create_variable(values[0] - values[1]);
    }

    VarPtr evaluate_multiply(const TypePtr&, const vector<Primitive>& values)
    {
        return create_variable(values[0] * values[1]);
    }

    VarPtr evaluate_divide(const TypePtr&, const vector<Primitive>& values)
    {
        return create_variable(values[0] / values[1]);
    }
}
