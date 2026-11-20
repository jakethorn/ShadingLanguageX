//
// Created by jaket on 04/06/2026.
//

#include "pybind.h"
#include "CompileOptions_bindings.h"
#include "Variable_bindings.h"
#include "GlobalInfo_bindings.h"
#include "Source_bindings.h"
#include "FieldInfo_bindings.h"
#include "ParameterInfo_bindings.h"
#include "OverloadInfo_bindings.h"
#include "TypeInfo_bindings.h"
#include "FunctionInfo_bindings.h"
#include "Reflection_bindings.h"
#include "compile_bindings.h"
#include "decompile_bindings.h"
#include "Decompiler_bindings.h"
#include "Macro_bindings.h"
#include "reflect_bindings.h"

PYBIND11_MODULE(_mxslc, m)
{
    m.doc() = "Python bindings for mxslc";
#ifdef MXSLCXX_PY_VERSION
    m.attr("__version__") = MXSLCXX_PY_VERSION;
#endif

    bind_macro(m);
    bind_variable(m);
    bind_create_variable(m);
    bind_compile_options(m);
    bind_global_info(m);
    bind_source(m);
    bind_field_info(m);
    bind_parameter_info(m);
    bind_overload_info(m);
    bind_function_info(m);
    bind_type_info(m);
    bind_reflection(m);
    bind_compile_functions(m);
    bind_reflect_functions(m);
    bind_decompile_functions(m);
    bind_decompiler(m);
}
