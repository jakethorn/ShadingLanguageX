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
#include "constants.h"

PYBIND11_MODULE(_mxslc, m)
{
    m.doc() = "Python bindings for mxslc";
    m.attr("DEFAULT_MTLX_VERSION") = DEFAULT_MTLX_VERSION;
#ifdef MXSLCXX_PY_VERSION
    m.attr("__version__") = MXSLCXX_PY_VERSION;
#endif

    bind_compile_options(m);
    bind_variable(m);
    bind_create_variable(m);
    bind_compile_functions(m);
    bind_decompile_functions(m);
    bind_decompiler(m);
    bind_macro(m);
}
