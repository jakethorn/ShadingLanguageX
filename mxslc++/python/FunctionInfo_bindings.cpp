//
// Created by jaket on 19/11/2026.
//

#include "FunctionInfo_bindings.h"

#include "reflection/FunctionInfo.h"

using namespace mxslc::reflection;

void bind_function_info(py::module_& m)
{
    py::class_<FunctionInfo>(m, "FunctionInfo")
        .def_readonly("name", &FunctionInfo::name)
        .def_readonly("overloads", &FunctionInfo::overloads)
        .def("__repr__", [](const FunctionInfo& info)
        {
            return "FunctionInfo(name='" + info.name + "')";
        });
}
