//
// Created by jaket on 04/11/2026.
//

#include "Reflection_bindings.h"

#include "reflection/Reflection.h"

using namespace mxslc::reflection;

void bind_reflection(py::module_& m)
{
    py::class_<Reflection>(m, "Reflection")
        .def_property_readonly("globals", &Reflection::globals)
        .def_property_readonly("type_names", &Reflection::type_names)
        .def_property_readonly("function_names", &Reflection::function_names);
}
