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
//     const opts = new mx.CompileOptions();
//     const mtlx = mx.compileSlxToMtlx('float x = add(1.0, 2.0);', opts);
//     const slx  = mx.decompileMtlxToSlx(mtlx);
//
//     // A root file that #includes (or #librarys) sibling files the
//     // browser only has as in-memory strings — e.g. dropped alongside
//     // it, never written to a real disk — can supply them as a plain
//     // object, keyed by the same relative path used in the directive:
//     const mtlx2 = mx.compileProjectToMtlx(
//         '#include "colors.mxsl"\nstandard_surface(base_color=RED);',
//         { 'colors.mxsl': 'const color3 RED = color3{1, 0, 0};' },
//         opts,
//     );
//     opts.delete();
//

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <exception>
#include <fstream>
#include <set>
#include <system_error>

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

    // Compile an SLX source string to a MaterialX (MTLX) XML string using
    // the given compile options, additionally making a set of virtual
    // sibling files available for #include / #library resolution.
    //
    // `files` is a plain JS object mapping a relative path — exactly as it
    // would appear inside a #include "..." or #library "..." directive —
    // to that file's source text. This exists because in a browser those
    // sibling files only ever exist as in-memory strings (e.g. dropped
    // alongside the root file), never as real paths on disk, so there is
    // nothing for the preprocessor's normal search-directory resolution
    // (LanguageSpecification.md's File Inclusion "additional directories
    // passed to the compiler" rule) to find on its own. Pass `undefined`,
    // `null`, or `{}` for `files` to compile a self-contained root with no
    // extra files, equivalent to compileSlxToMtlx.
    //
    // Each entry is staged into a fixed scratch directory inside the WASM's
    // in-memory filesystem, which is then added as a compile search
    // directory; the scratch directory is cleared at the START of every
    // call (and best-effort cleaned up afterwards too), so files staged by
    // an earlier, unrelated compile can never leak into a later one.
    std::string compile_project_to_mtlx(const std::string& root_source,
                                        const ems::val& files,
                                        mxslc::CompileOptions opts)
    {
        static const fs::path SCRATCH_DIR = "/mxsl_project";
        std::error_code ec;

        try
        {
            // Best-effort: a not-yet-existing scratch dir is not an error.
            fs::remove_all(SCRATCH_DIR, ec);
            fs::create_directories(SCRATCH_DIR);

            if (!files.isNull() && !files.isUndefined())
            {
                const ems::val keys = ems::val::global("Object").call<ems::val>("keys", files);
                const unsigned count = keys["length"].as<unsigned>();
                for (unsigned i = 0; i < count; ++i)
                {
                    const std::string rel_path = keys[i].as<std::string>();
                    const std::string content = files[rel_path].as<std::string>();

                    // Keep every staged file inside SCRATCH_DIR: this is only
                    // an in-memory, per-call sandbox (never the real host
                    // filesystem), but rejecting an absolute path or a '..'
                    // segment keeps staging predictable instead of silently
                    // writing somewhere unexpected inside that sandbox.
                    if (fs::path(rel_path).is_absolute() || rel_path.find("..") != std::string::npos)
                        throw std::runtime_error("Invalid virtual file path (must be relative, no '..'): " + rel_path);

                    const fs::path dest = SCRATCH_DIR / rel_path;
                    fs::create_directories(dest.parent_path());

                    std::ofstream out(dest, std::ios::binary | std::ios::trunc);
                    if (!out)
                        throw std::runtime_error("Could not stage virtual file: " + rel_path);
                    out << content;
                }
            }

            opts.add_search_directory(SCRATCH_DIR);
            const std::string result = mxslc::compile_to_string(root_source, opts);

            fs::remove_all(SCRATCH_DIR, ec);
            return result;
        }
        catch (const std::exception& e)
        {
            fs::remove_all(SCRATCH_DIR, ec);
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
                mxslc::load_materialx_library(opts.version, opts.search_directories());

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
    ems::function("compileProjectToMtlx", &compile_project_to_mtlx);
    ems::function("decompileMtlxToSlx", &decompile_mtlx_to_slx);

    ems::register_vector<std::string>("StringVector");
    ems::function("getMtlxDefinitionNames", &get_mtlx_definition_names);
}
