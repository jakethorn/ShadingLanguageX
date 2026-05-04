//
// Created by jaket on 30/12/2025.
//

#ifndef FENNEC_LOAD_MTLX_H
#define FENNEC_LOAD_MTLX_H

#include <MaterialXCore/Document.h>

#include "utils/common.h"

void load_library(const mx::DocumentPtr &doc);
void load_library(const fs::path& filepath);
mx::DocumentPtr get_materialx_library(const string& version);

#endif //FENNEC_LOAD_MTLX_H
