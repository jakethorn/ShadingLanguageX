//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/separate.h"

#include "Primitive.h"
#include "utils/primitive_utils.h"
#include "runtime/interface.h"

namespace mxslc::serialize
{
    VarPtr evaluate_separate2(const TypePtr&, const vector<Primitive>& values)
    {
        return create_variable(
            primitive_utils::separate(values[0])
        );
    }

    VarPtr evaluate_separate3(const TypePtr&, const vector<Primitive>& values)
    {
        return create_variable(
            primitive_utils::separate(values[0])
        );
    }

    VarPtr evaluate_separate4(const TypePtr&, const vector<Primitive>& values)
    {
        return create_variable(
            primitive_utils::separate(values[0])
        );
    }
}
