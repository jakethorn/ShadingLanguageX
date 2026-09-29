//
// Created by jaket on 24/06/2026.
//

#ifndef MXSLC_DECOMPILE_BINDINGS_H
#define MXSLC_DECOMPILE_BINDINGS_H

#include "pybind.h"

void bind_decompile_options(py::module_& m);
void bind_decompile_functions(py::module_& m);

#endif //MXSLC_DECOMPILE_BINDINGS_H
