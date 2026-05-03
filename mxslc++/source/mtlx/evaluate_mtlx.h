//
// Created by jaket on 10/04/2026.
//

#ifndef MXSLC_EVALUATE_MTLX_H
#define MXSLC_EVALUATE_MTLX_H

#include "utils/common.h"

class Parameter;

VarPtr evaluate_now(const string& node_name, const vector<pair<const Parameter&, VarPtr>>& arg_values);

#endif //MXSLC_EVALUATE_MTLX_H
