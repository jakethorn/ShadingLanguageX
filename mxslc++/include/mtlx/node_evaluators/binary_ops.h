//
// Created by jaket on 28/05/2026.
//

#ifndef MXSLC_BINARY_OPS_H
#define MXSLC_BINARY_OPS_H

#include "utils/common.h"

VarPtr evaluate_add(const TypePtr&, const vector<BasicValuePtr>& values);
VarPtr evaluate_subtract(const TypePtr&, const vector<BasicValuePtr>& values);
VarPtr evaluate_multiply(const TypePtr&, const vector<BasicValuePtr>& values);
VarPtr evaluate_divide(const TypePtr&, const vector<BasicValuePtr>& values);

#endif //MXSLC_BINARY_OPS_H