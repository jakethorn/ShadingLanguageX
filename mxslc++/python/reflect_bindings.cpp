//
// Created by jaket on 04/11/2026.
//

#include "reflect_bindings.h"

#include "reflection/reflect.h"
#include "CompileOptions.h"

using namespace mxslc::reflection;

void bind_reflect_functions(py::module_& m)
{
    m.def(
        "reflect_from_file",
        py::overload_cast<const fs::path&>(&reflect),
        py::arg("src_path")
    );

    m.def(
        "reflect_from_file",
        py::overload_cast<const fs::path&, const CompileOptions&>(&reflect),
        py::arg("src_path"),
        py::arg("opts")
    );

    m.def(
        "reflect_from_string",
        py::overload_cast<const string&>(&reflect),
        py::arg("source")
    );

    m.def(
        "reflect_from_string",
        py::overload_cast<const string&, const CompileOptions&>(&reflect),
        py::arg("source"),
        py::arg("opts")
    );
}
