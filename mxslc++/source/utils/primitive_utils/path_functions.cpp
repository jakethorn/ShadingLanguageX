//
// Created by jaket on 26/07/2026.
//

#include "utils/primitive_utils.h"

namespace mxslc::primitive_utils
{
    Primitive exists(const Primitive& path)
    {
        return fs::exists(path.cast<fs::path>());
    }

    Primitive is_dir(const Primitive& path)
    {
        return fs::is_directory(path.cast<fs::path>());
    }

    Primitive is_file(const Primitive& path)
    {
        return fs::is_regular_file(path.cast<fs::path>());
    }
}
