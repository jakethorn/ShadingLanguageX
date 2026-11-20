//
// Created by jaket on 19/11/2026.
//

#include "TypeInfo_bindings.h"

#include "utils.h"
#include "reflection/TypeInfo.h"

using namespace mxslc::reflection;

void bind_type_info(py::module_& m)
{
    py::class_<TypeInfo>(m, "TypeInfo")
        .def_readonly("name", &TypeInfo::name)
        .def_readonly("is_alias", &TypeInfo::is_alias)
        .def_readonly("source", &TypeInfo::source)
        .def_property_readonly("aliased_type", [](const TypeInfo& info) { return utils::to_py_str_or_none(info.aliased_type); })
        .def_readonly("fields", &TypeInfo::fields)
        .def_readonly("methods", &TypeInfo::methods)
        .def_property_readonly("path", [](const TypeInfo& info) { return utils::to_py_path_or_none(info.path); })
        .def_property_readonly("line", [](const TypeInfo& info) { return utils::to_py_line_or_none(info.line); })
        .def("__repr__", [](const py::object& info)
        {
            return py::str("TypeInfo(name={!r}, is_alias={!r}, source={!r}, aliased_type={!r}, path={!r}, line={!r})").format(
                info.attr("name"), info.attr("is_alias"), info.attr("source"), info.attr("aliased_type"), info.attr("path"),
                info.attr("line"));
        });
}
