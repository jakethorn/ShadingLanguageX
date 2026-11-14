//
// Created by jaket on 19/06/2026.
//

#ifndef MXSLC_DECOMPILE_H
#define MXSLC_DECOMPILE_H

#include <MaterialXCore/Document.h>

#include "common.h"

namespace mxslc::decompile
{
    string decompile_to_string(const fs::path& src_path);
    // MaterialX source, or the path of a file if the string does not start with an element, see Decompiler
    string decompile_to_string(const string& source);
    string decompile_to_string(const mx::DocumentPtr& document);

    fs::path decompile_to_file(const fs::path& src_path, const optional<fs::path>& dst_path = std::nullopt);
    fs::path decompile_to_file(const fs::path& src_path, const fs::path& dst_path);
    fs::path decompile_to_file(const string& source, const fs::path& dst_path);
    fs::path decompile_to_file(const mx::DocumentPtr& document, const fs::path& dst_path);
}

#endif //MXSLC_DECOMPILE_H
