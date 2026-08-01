//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_SERIALIZE_CONSTEXPR_H
#define MXSLC_SERIALIZE_CONSTEXPR_H

#include "common.h"

namespace mxslc::serialize
{
    VarPtr serialize_constexpr(const TypePtr& node_type, const string& node_name, const ParameterValues& input_values);
}

#endif //MXSLC_SERIALIZE_CONSTEXPR_H
