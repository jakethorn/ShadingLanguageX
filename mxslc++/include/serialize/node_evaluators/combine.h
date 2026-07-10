//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_COMBINE_H
#define MXSLC_COMBINE_H

#include "common.h"
#include "serialize/values/interface.h"

namespace mxslc::optimisations
{
    VarPtr evaluate_combine2(const TypePtr& type, const vector<BasicValuePtr>& values);
    VarPtr evaluate_combine3(const TypePtr& type, const vector<BasicValuePtr>& values);
    VarPtr evaluate_combine4(const TypePtr& type, const vector<BasicValuePtr>& values);
}

#endif //MXSLC_COMBINE_H
