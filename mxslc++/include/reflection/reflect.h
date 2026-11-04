//
// Created by jaket on 04/11/2026.
//

#ifndef MXSLC_REFLECT_H
#define MXSLC_REFLECT_H

#include "common.h"
#include "Reflection.h"
#include "CompileOptions.h"

namespace mxslc::reflection
{
    Reflection reflect(const fs::path& src_path);
    Reflection reflect(const fs::path& src_path, const CompileOptions& opts);
    Reflection reflect(const string& source);
    Reflection reflect(const string& source, const CompileOptions& opts);
}

#endif //MXSLC_REFLECT_H
