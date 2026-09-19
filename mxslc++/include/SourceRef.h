#ifndef MXSLC_SOURCEREF_H
#define MXSLC_SOURCEREF_H

#include "common.h"

namespace mxslc
{
    // A source resolved from the CompileOptions search path.
    //
    // `contents` is set when the source was supplied in memory (see
    // CompileOptions::add_source) rather than read from a file on disk. In that
    // case `path` is the name as written in the directive, so it is only used
    // for diagnostics and include-cycle detection, never for file access.
    struct SourceRef
    {
        fs::path path;
        optional<string> contents;

        bool is_in_memory() const { return contents.has_value(); }
    };
}

#endif //MXSLC_SOURCEREF_H
