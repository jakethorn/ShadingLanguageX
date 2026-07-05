//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_SCAN_H
#define FENNEC_SCAN_H

#include "utils/common.h"
#include "../include/Token.h"

vector<Token> scan_string(string_view text);
vector<Token> scan_file(const fs::path& src_path);

#endif //FENNEC_SCAN_H
