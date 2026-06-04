//
// Created by jaket on 02/11/2025.
//

#ifndef FENNEC_IO_UTILS_H
#define FENNEC_IO_UTILS_H

#include "common.h"

string read_file(const fs::path& src_path);
void save_file(const fs::path& dst_path, const string& text);
fs::path get_executable_dir();
fs::path get_python_module_dir();

#endif //FENNEC_IO_UTILS_H
