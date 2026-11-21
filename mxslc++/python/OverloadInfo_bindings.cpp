//
// Created by jaket on 20/11/2026.
//

#include "OverloadInfo_bindings.h"

#include "utils.h"
#include "reflection/OverloadInfo.h"

using namespace mxslc::reflection;

void bind_overload_info(py::module_& m)
{
    py::class_<OverloadInfo>(m, "OverloadInfo")
        .def_readonly("parameters", &OverloadInfo::parameters)
        .def_readonly("return_type", &OverloadInfo::return_type)
        .def_property_readonly("template_type", [](const OverloadInfo& info) { return utils::to_py_str_or_none(info.template_type); })
        .def_readonly("modifiers", &OverloadInfo::modifiers)
        .def_readonly("source", &OverloadInfo::source)
        .def_property_readonly("class_type", [](const OverloadInfo& info) { return utils::to_py_str_or_none(info.class_type); })
        .def_property_readonly("path", [](const OverloadInfo& info) { return utils::to_py_path_or_none(info.path); })
        .def_property_readonly("line", [](const OverloadInfo& info) { return utils::to_py_line_or_none(info.line); })
        .def("__repr__", [](const py::object& info)
        {
            return py::str("OverloadInfo(return_type={!r}, parameters={!r}, modifiers={!r}, source={!r})").format(
                info.attr("return_type"), info.attr("parameters"), info.attr("modifiers"), info.attr("source"));
        });
}
