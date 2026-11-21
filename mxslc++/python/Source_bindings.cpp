//
// Created by jaket on 20/11/2026.
//

#include "Source_bindings.h"

#include "reflection/Source.h"

using namespace mxslc::reflection;

void bind_source(py::module_& m)
{
    py::enum_<Source>(m, "Source")
        .value("MaterialX", Source::MaterialX)
        .value("SlxStandardLibrary", Source::SlxStandardLibrary)
        .value("UserDefined", Source::UserDefined);
}
