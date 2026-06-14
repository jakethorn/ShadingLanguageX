//
// Created by jaket on 12/06/2026.
//

#ifndef MXSLC_PYTHON_COMMON_H
#define MXSLC_PYTHON_COMMON_H

#include <ciso646>
#include <memory>
#include <optional>
#include <variant>
#include <string>
#include <vector>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl/filesystem.h>

#include <MaterialXCore/Types.h>

#include "compile.h"
#include "CompileOptions.h"
#include "primitive_t.h"

using std::unique_ptr;
using std::shared_ptr;
using std::weak_ptr;
using std::optional;
using std::string;
using std::string_view;
using std::vector;
using std::tuple;
using std::pair;
using std::variant;
using std::unordered_map;
using std::unordered_set;

namespace py = pybind11;

namespace mx = MaterialX;

#endif //MXSLC_PYTHON_COMMON_H
