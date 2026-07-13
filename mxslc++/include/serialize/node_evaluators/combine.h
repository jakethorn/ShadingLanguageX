//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_COMBINE_H
#define MXSLC_COMBINE_H

#include "common.h"

namespace mxslc
{
    class Primitive;
}

namespace mxslc::serialize
{
    VarPtr evaluate_combine2(const TypePtr& type, const vector<Primitive>& values);
    VarPtr evaluate_combine3(const TypePtr& type, const vector<Primitive>& values);
    VarPtr evaluate_combine4(const TypePtr& type, const vector<Primitive>& values);
}

#endif //MXSLC_COMBINE_H
