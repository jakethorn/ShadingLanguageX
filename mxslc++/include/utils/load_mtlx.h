//
// Created by jaket on 30/12/2025.
//

#ifndef FENNEC_LOAD_MTLX_H
#define FENNEC_LOAD_MTLX_H

#include <MaterialXCore/Document.h>

#include "common.h"

namespace mxslc
{
    void add_library_to_scope(const mx::DocumentPtr& doc);
    void add_library_to_scope(const fs::path& filepath);

    mx::DocumentPtr load_materialx_library(const string& version, const vector<fs::path>& include_dirs);
    void load_materialx_library(const string& version, const vector<fs::path>& include_dirs, const mx::DocumentPtr& doc);
}

#endif //FENNEC_LOAD_MTLX_H
