//
// Created by jaket on 20/11/2026.
//

#include "FieldInfo_bindings.h"

#include "reflection/FieldInfo.h"

using namespace mxslc::reflection;

void bind_field_info(py::module_& m)
{
    py::class_<FieldInfo>(m, "FieldInfo")
        .def_readonly("name", &FieldInfo::name)
        .def_readonly("type", &FieldInfo::type)
        .def("__repr__", [](const FieldInfo& info)
        {
            return "FieldInfo(name='" + info.name + "', type='" + info.type + "')";
        });
}
