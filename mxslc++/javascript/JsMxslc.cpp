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
//     const opts = = new mx.CompileOptions();
//     const mtlx = mx.compileSlxToMtlx('float x = add(1.0, 2.0);');
//     const slx  = mx.decompileMtlxToSlx(mtlx);
//     opts.delete();
//

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <exception>
#include <set>

#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "utils/load_mtlx.h"

#include "common.h"

namespace ems = emscripten;

namespace
{
    // Convert a C++ exception into a proper JavaScript Error and throw it.
    // Embind's default translation of a C++ exception crossing the boundary
    // surfaces an opaque value with no usable `.message` on the JS side, so we
    // catch and rethrow as a real Error carrying the exception's text.
    [[noreturn]] void rethrow_as_js_error(const std::exception& e, const char* name)
    {
        ems::val err = ems::val::global("Error").new_(std::string(e.what()));
        err.set("name", std::string(name));
        err.throw_();
    }

    // Compile an SLX source string to a MaterialX (MTLX) XML string using
    // the given compile options.
    std::string compile_slx_to_mtlx(const std::string& source,
                                    const mxslc::CompileOptions& opts)
    {
        try
        {
            return mxslc::compile_to_string(source, opts);
        }
        catch (const std::exception& e)
        {
            rethrow_as_js_error(e, "CompileError");
        }
    }

    // Decompile a MaterialX (MTLX) XML string to an SLX source string.
    std::string decompile_mtlx_to_slx(const std::string& source)
    {
        try
        {
            return mxslc::decompile::decompile_to_string(source);
        }
        catch (const std::exception& e)
        {
            rethrow_as_js_error(e, "Error");
        }
    }

    // Return the sorted set of MaterialX node-definition category names from
    // the loaded standard library. This reuses mxslc's existing MaterialX
    // library loader (the same one the compile path uses), so the library is
    // resolved against the preloaded libraries/ folder in the WASM filesystem.
    std::vector<std::string> get_mtlx_definition_names()
    {
        try
        {
            mxslc::CompileOptions opts;
            opts.add_default_search_directories();

            const mx::DocumentPtr doc =
                mxslc::get_materialx_library(opts.version, opts.search_directories());

            std::set<std::string> names;
            for (const mx::NodeDefPtr& nd : doc->getNodeDefs())
            {
                // Skip non-default versioned nodedefs, matching how mxslc loads them.
                if (nd->hasVersionString() && !nd->getDefaultVersion())
                    continue;
                names.insert(nd->getNodeString());
            }
            return std::vector<std::string>(names.begin(), names.end());
        }
        catch (const std::exception& e)
        {
            rethrow_as_js_error(e, "Error");
        }
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
    ems::function("decompileMtlxToSlx", &decompile_mtlx_to_slx);

    ems::register_vector<std::string>("StringVector");
    ems::function("getMtlxDefinitionNames", &get_mtlx_definition_names);
}
