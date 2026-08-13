//
// Created by mxslc contributors
//
// Emscripten (WebAssembly) bindings for the mxslc compile / decompile
// pipeline. This exposes a small, string-oriented JavaScript API so that SLX
// source strings can be compiled to MaterialX (MTLX) XML strings and MTLX XML
// strings can be decompiled back to SLX source strings.
//
// The module is exported as an ES6 module that can be consumed from Node.js or
// a browser:
//
//     import Mxslc from './JsMxslc.js';
//     const mx = await Mxslc();
//     const mtlx = mx.compileSlxToMtlx('float x = add(1.0, 2.0);');
//     const slx  = mx.decompileMtlxToSlx(mtlx);
//

#include <emscripten/bind.h>

#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"

#include "common.h"

namespace ems = emscripten;

namespace
{
    // Compile an SLX source string to a MaterialX (MTLX) XML string.
    std::string compile_slx_to_mtlx(const std::string& source)
    {
        return mxslc::compile_to_string(source);
    }

    // Compile an SLX source string to a MaterialX (MTLX) XML string using
    // the given compile options.
    std::string compile_slx_to_mtlx_with_options(const std::string& source,
                                                 const mxslc::CompileOptions& opts)
    {
        return mxslc::compile_to_string(source, opts);
    }

    // Decompile a MaterialX (MTLX) XML string to an SLX source string.
    std::string decompile_mtlx_to_slx(const std::string& source)
    {
        return mxslc::decompile::decompile_to_string(source);
    }
}

EMSCRIPTEN_BINDINGS(mxslc)
{
    ems::class_<mxslc::CompileOptions>("CompileOptions")
        .constructor<>()
        .property("version", &mxslc::CompileOptions::version)
        .property("reduceGraph", &mxslc::CompileOptions::reduce_graph)
        .property("errorOnMissingGlobals", &mxslc::CompileOptions::error_on_missing_globals)
        .property("errorOnUnusedGlobals", &mxslc::CompileOptions::error_on_unused_globals);

    ems::function("compileSlxToMtlx", &compile_slx_to_mtlx);
    ems::function("compileSlxToMtlxWithOptions", &compile_slx_to_mtlx_with_options);
    ems::function("decompileMtlxToSlx", &decompile_mtlx_to_slx);
}
