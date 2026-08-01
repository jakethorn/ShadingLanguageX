//
// Created by jaket on 24/06/2026.
//

#include "Variable_bindings.h"

#include "utils.h"
#include "runtime/Variable.h"

void bind_variable(py::module_& m)
{
    py::class_<Variable, VarPtr>(m, "Variable")
        .def_property_readonly("value", &utils::to_py_variable);
}

void bind_create_variable(py::module_& m)
{
    m.def(
        "create_variable",
        &utils::to_cpp_variable,
        py::arg("value")
    );
}
