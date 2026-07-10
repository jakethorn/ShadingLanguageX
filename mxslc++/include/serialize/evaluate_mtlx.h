//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_EVALUATE_MTLX_H
#define MXSLC_EVALUATE_MTLX_H

#include "common.h"

namespace mxslc::optimisations
{
    VarPtr evaluate_now(const TypePtr& node_type, const string& node_name, const ParameterValues& input_values);
}

#endif //MXSLC_EVALUATE_MTLX_H
