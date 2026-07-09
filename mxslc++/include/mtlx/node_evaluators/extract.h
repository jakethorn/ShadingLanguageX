//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_EXTRACT_H
#define MXSLC_EXTRACT_H

#include "common.h"
#include "values/interface.h"

namespace mxslc::optimisations
{
    VarPtr evaluate_extract(const TypePtr&, const vector<BasicValuePtr>& values);
}

#endif //MXSLC_EXTRACT_H
