//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_SEPARATE_H
#define MXSLC_SEPARATE_H

#include "common.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize
{
    VarPtr evaluate_separate2(const TypePtr& type, const vector<BasicValuePtr>& values);
    VarPtr evaluate_separate3(const TypePtr& type, const vector<BasicValuePtr>& values);
    VarPtr evaluate_separate4(const TypePtr& type, const vector<BasicValuePtr>& values);
}

#endif //MXSLC_SEPARATE_H
