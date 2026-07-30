//
// Created by jaket on 04/06/2026.
//

#include "pybind.h"
#include "CompileOptions_bindings.h"
#include "Variable_bindings.h"
#include "compile_bindings.h"
#include "decompile_bindings.h"
#include "Decompiler_bindings.h"
#include "Macro_bindings.h"

PYBIND11_MODULE(_mxslc, m)
{
    m.doc() = "Python bindings for mxslc";

    bind_compile_options(m);
    bind_variable(m);
    bind_create_variable(m);
    bind_compile_functions(m);
    bind_decompile_functions(m);
    bind_decompiler(m);
    bind_macro(m);
}
