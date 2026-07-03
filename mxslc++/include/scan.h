//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_SCAN_H
#define FENNEC_SCAN_H

#include "common.h"
#include "Token.h"

namespace mxslc
{
    vector<Token> scan_string(string_view text);
    vector<Token> scan_file(const fs::path& src_path);
}

#endif //FENNEC_SCAN_H
