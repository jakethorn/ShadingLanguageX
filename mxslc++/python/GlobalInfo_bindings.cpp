//
// Created by jaket on 30/10/2026.
//

#include "GlobalInfo_bindings.h"

#include "utils.h"
#include "reflection/GlobalInfo.h"
#include "runtime/Type.h"

using namespace mxslc::reflection;

void bind_global_info(py::module_& m)
{
    py::class_<GlobalInfo>(m, "GlobalInfo")
        .def_readonly("name", &GlobalInfo::name)
        .def_property_readonly("type", [](const GlobalInfo& global) { return global.type->to_string(); })
        .def_property_readonly("default_value", [](const GlobalInfo& global) { return utils::to_py_variable(global.default_value); })
        .def("__repr__", [](const GlobalInfo& global)
        {
            return "GlobalInfo(name='" + global.name + "', type='" + global.type->to_string() + "')";
        });
}
