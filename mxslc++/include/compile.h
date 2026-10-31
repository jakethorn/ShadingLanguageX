//
// Created by jaket on 02/01/2026.
//

#ifndef MXSLC_COMPILE_H
#define MXSLC_COMPILE_H

#include <MaterialXCore/Document.h>

#include "common.h"
#include "GlobalVariable.h"

namespace mxslc
{
    struct CompileOptions;
    
    mx::DocumentPtr compile_to_document(const fs::path& src_path);
    mx::DocumentPtr compile_to_document(const fs::path& src_path, const CompileOptions& opts);
    string compile_to_string(const fs::path& src_path);
    string compile_to_string(const fs::path& src_path, const CompileOptions& opts);
    fs::path compile_to_file(const fs::path& src_path);
    fs::path compile_to_file(const fs::path& src_path, const CompileOptions& opts);
    fs::path compile_to_file(const fs::path& src_path, const fs::path& dst_path);

    mx::DocumentPtr compile_to_document(const string& source);
    mx::DocumentPtr compile_to_document(const string& source, const CompileOptions& opts);
    string compile_to_string(const string& source);
    string compile_to_string(const string& source, const CompileOptions& opts);
    fs::path compile_to_file(const string& source, const CompileOptions& opts);
    fs::path compile_to_file(const string& source, const fs::path& dst_path);

    // Runs the program without global values and returns the `global` variables it declares, in declaration order.
    vector<GlobalVariable> get_globals(const fs::path& src_path);
    vector<GlobalVariable> get_globals(const fs::path& src_path, const CompileOptions& opts);
    vector<GlobalVariable> get_globals(const string& source);
    vector<GlobalVariable> get_globals(const string& source, const CompileOptions& opts);
}

#endif //MXSLC_COMPILE_H
