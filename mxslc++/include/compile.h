//
// Created by jaket on 02/01/2026.
//

#ifndef MXSLC_COMPILE_H
#define MXSLC_COMPILE_H

#include <string>
#include <filesystem>
#include <MaterialXCore/Document.h>

#include "parse_args.h"

namespace mxslc
{
    void compile(const std::filesystem::path& src_path, const std::filesystem::path& dst_path);
    void compile(const std::filesystem::path& src_path);

    //std::string compile_to_string(const std::string& source, const CompileOptions* opts = nullptr);
    //std::string compile_to_string(const std::filesystem::path& src_path, const CompileOptions* opts = nullptr);

    //MaterialX::DocumentPtr compile_to_document(const std::string& source, const CompileOptions* opts = nullptr);
    //MaterialX::DocumentPtr compile_to_document(const std::filesystem::path& src_path, const CompileOptions* opts = nullptr);

    //void compile_to_file(const std::string& source, const std::filesystem::path& dst_path, const CompileOptions* opts = nullptr);
    //void compile_to_file(const std::filesystem::path& src_path, const std::filesystem::path& dst_path, const CompileOptions* opts = nullptr);
    //std::filesystem::path compile_to_file(const std::filesystem::path& src_path, const CompileOptions* opts = nullptr);
}

#endif //MXSLC_COMPILE_H
