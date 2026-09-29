//
// Created by jaket on 19/06/2026.
//

#include "decompile/decompile.h"

#include "decompile/Decompiler.h"
#include "common.h"
#include "utils/io_utils.h"

namespace mxslc::decompile
{
    string decompile_to_string(const fs::path& src_path, const DecompileOptions& options)
    {
        return Decompiler{src_path, options}.decompile_document();
    }

    string decompile_to_string(const string& source, const DecompileOptions& options)
    {
        return Decompiler{source, options}.decompile_document();
    }

    string decompile_to_string(const mx::DocumentPtr& document, const DecompileOptions& options)
    {
        return Decompiler{document, options}.decompile_document();
    }

    fs::path decompile_to_file(const fs::path& src_path, const optional<fs::path>& dst_path, const DecompileOptions& options)
    {
        const fs::path tmp_path = dst_path.value_or(fs::path{src_path}.replace_extension(".mxsl"));
        return decompile_to_file(src_path, tmp_path, options);
    }

    fs::path decompile_to_file(const fs::path& src_path, const fs::path& dst_path, const DecompileOptions& options)
    {
        io_utils::save_file(dst_path, decompile_to_string(src_path, options));
        return dst_path;
    }

    fs::path decompile_to_file(const string& source, const fs::path& dst_path, const DecompileOptions& options)
    {
        io_utils::save_file(dst_path, decompile_to_string(source, options));
        return dst_path;
    }

    fs::path decompile_to_file(const mx::DocumentPtr& document, const fs::path& dst_path, const DecompileOptions& options)
    {
        io_utils::save_file(dst_path, decompile_to_string(document, options));
        return dst_path;
    }
}
