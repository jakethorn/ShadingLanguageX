//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_CONVERT_H
#define MXSLC_CONVERT_H

#include "common.h"
#include "values/interface.h"

namespace mxslc::optimisations
{
    VarPtr evaluate_convert(const TypePtr& type, const vector<BasicValuePtr>& values);
}

#endif //MXSLC_CONVERT_H
