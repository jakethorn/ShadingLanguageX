//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_SCAN_H
#define FENNEC_SCAN_H

#include "common.h"
#include "SourceRef.h"
#include "Token.h"

namespace mxslc
{
    vector<Token> scan_string(string_view text, const optional<fs::path>& src_path = std::nullopt);
    vector<Token> scan_file(const fs::path& src_path);
    vector<Token> scan_source(const SourceRef& source);
}

#endif //FENNEC_SCAN_H
