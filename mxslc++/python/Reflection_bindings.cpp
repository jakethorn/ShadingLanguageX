//
// Created by jaket on 04/11/2026.
//

#include "Reflection_bindings.h"

#include "reflection/Reflection.h"

using namespace mxslc::reflection;

void bind_reflection(py::module_& m)
{
    py::class_<Reflection>(m, "Reflection")
        .def_property_readonly("globals", &Reflection::globals);
}
