//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_PYTHON_UTILS_H
#define MXSLC_PYTHON_UTILS_H

#include "pybind.h"

#include "Primitive.h"
#include "preprocess/Macro.h"

namespace utils
{
    string get_type_name(const py::handle& py_obj);

    vector<float> to_cpp_float_sequence(const py::handle& py_obj);
    Primitive     to_cpp_primitive(const py::handle& py_obj);
    VarPtr        to_cpp_variable(const py::handle& py_obj);
    Macro         to_cpp_macro(const py::handle& py_obj);

    py::object to_py_primitive(const Primitive& primitive);
    py::object to_py_variable(const VarPtr& var);

    bool is_primitive_type(const py::handle& py_obj);
}

#endif //MXSLC_PYTHON_UTILS_H
