//
// Created by jaket on 30/12/2025.
//

#ifndef FENNEC_LOAD_MTLX_H
#define FENNEC_LOAD_MTLX_H

#include <MaterialXCore/Document.h>
#include "utils/common.h"
#include "runtime/Runtime.h"

void load_library(const Runtime& runtime, const mx::DocumentPtr &doc);
void load_library(const Runtime& runtime, const fs::path& filepath);
void load_materialx_library(const Runtime& runtime, const string& version);

#endif //FENNEC_LOAD_MTLX_H
