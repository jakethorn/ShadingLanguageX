//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_EXTRACT_H
#define MXSLC_EXTRACT_H

#include "common.h"

namespace mxslc
{
    class Primitive;
}

namespace mxslc::serialize
{
    VarPtr evaluate_extract(const TypePtr&, const vector<Primitive>& values);
}

#endif //MXSLC_EXTRACT_H
