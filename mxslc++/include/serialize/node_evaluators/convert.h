//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_CONVERT_H
#define MXSLC_CONVERT_H

#include "common.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize
{
    VarPtr evaluate_convert(const TypePtr& type, const vector<Primitive>& values);
}

#endif //MXSLC_CONVERT_H
