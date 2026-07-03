//
// Created by jaket on 29/07/2026.
//

#include "Macro_bindings.h"

#include "preprocess/Macro.h"

void bind_macro(py::module_& m)
{
    py::class_<Macro>(m, "Macro")
        .def(py::init<string>(), py::arg("name"))
        .def(py::init<string, const string&>(), py::arg("name"), py::arg("body"))
        .def_property_readonly("name", &Macro::name);
}
