//
// Created by jaket on 28/05/2026.
//

#include "serialize/node_evaluators/combine.h"

#include "Primitive.h"
#include "utils/primitive_utils.h"
#include "runtime/interface.h"

namespace mxslc::serialize
{
    VarPtr evaluate_combine2(const TypePtr& type, const vector<Primitive>& values)
    {
        return create_variable(
            primitive_utils::combine(values, type)
        );
    }

    VarPtr evaluate_combine3(const TypePtr& type, const vector<Primitive>& values)
    {
        return create_variable(
            primitive_utils::combine(values, type)
        );
    }

    VarPtr evaluate_combine4(const TypePtr& type, const vector<Primitive>& values)
    {
        return create_variable(
            primitive_utils::combine(values, type)
        );
    }
}
