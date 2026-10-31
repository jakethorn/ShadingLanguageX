//
// Created by jaket on 30/10/2026.
//

#include "GlobalVariable_bindings.h"

#include "utils.h"
#include "GlobalVariable.h"
#include "runtime/Type.h"

void bind_global_variable(py::module_& m)
{
    py::class_<GlobalVariable>(m, "GlobalVariable")
        .def_readonly("name", &GlobalVariable::name)
        .def_property_readonly("type", [](const GlobalVariable& global) { return global.type->to_string(); })
        .def_property_readonly("value", [](const GlobalVariable& global) { return utils::to_py_variable(global.value); })
        .def("__repr__", [](const GlobalVariable& global)
        {
            return "GlobalVariable(name='" + global.name + "', type='" + global.type->to_string() + "')";
        });
}
