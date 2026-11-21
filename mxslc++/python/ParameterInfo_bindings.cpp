//
// Created by jaket on 20/11/2026.
//

#include "ParameterInfo_bindings.h"

#include "utils.h"
#include "reflection/ParameterInfo.h"

using namespace mxslc::reflection;

void bind_parameter_info(py::module_& m)
{
    py::class_<ParameterInfo>(m, "ParameterInfo")
        .def_readonly("name", &ParameterInfo::name)
        .def_readonly("type", &ParameterInfo::type)
        .def_property_readonly("default_expression", [](const ParameterInfo& info) { return utils::to_py_str_or_none(info.default_expression); })
        .def_readonly("modifiers", &ParameterInfo::modifiers)
        .def("__repr__", [](const py::object& info)
        {
            return py::str("ParameterInfo(name={!r}, type={!r}, default_expression={!r}, modifiers={!r})").format(
                info.attr("name"), info.attr("type"), info.attr("default_expression"), info.attr("modifiers"));
        });
}
