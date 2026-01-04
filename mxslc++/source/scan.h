//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_SCAN_H
#define FENNEC_SCAN_H

#include "utils/common.h"
#include "Token.h"

[[nodiscard]] vector<Token> sscan(string_view text);
[[nodiscard]] vector<Token> fscan(const fs::path& filepath);

#endif //FENNEC_SCAN_H
