//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_PYTHON_UTILS_H
#define MXSLC_PYTHON_UTILS_H

#include "common.h"

namespace utils
{
    vector<float> marshall_float_sequence(const py::handle& py_obj);
    mxslc::primitive_t marshall_primitive(const py::handle& py_obj);
}

#endif //MXSLC_PYTHON_UTILS_H
