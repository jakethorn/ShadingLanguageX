//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_UTILS_H
#define FENNEC_UTILS_H

#include "common.h"

string read_file(const fs::path& filepath);
void save_file(const fs::path& filepath, const string& text);
fs::path get_libraries_dir();

#endif //FENNEC_UTILS_H
