//
// Created by jaket on 19/06/2026.
//

#include "decompile.h"

#include "Decompiler.h"
#include "utils/common.h"
#include "utils/io_utils.h"

string mxslc::decompile_to_string(const fs::path& src_path)
{
    return Decompiler{src_path}.decompile_document();
}

string mxslc::decompile_to_string(const string& source)
{
    return Decompiler{source}.decompile_document();
}

string mxslc::decompile_to_string(const mx::DocumentPtr& document)
{
    return Decompiler{document}.decompile_document();
}

fs::path mxslc::decompile_to_file(const fs::path& src_path, const optional<fs::path>& dst_path)
{
    const fs::path tmp_path = dst_path.value_or(fs::path{src_path}.replace_extension(".mxsl"));
    return decompile_to_file(src_path, tmp_path);
}

fs::path mxslc::decompile_to_file(const fs::path& src_path, const fs::path& dst_path)
{
    save_file(dst_path, decompile_to_string(src_path));
    return dst_path;
}

fs::path mxslc::decompile_to_file(const string& source, const fs::path& dst_path)
{
    save_file(dst_path, decompile_to_string(source));
    return dst_path;
}

fs::path mxslc::decompile_to_file(const mx::DocumentPtr& document, const fs::path& dst_path)
{
    save_file(dst_path, decompile_to_string(document));
    return dst_path;
}
