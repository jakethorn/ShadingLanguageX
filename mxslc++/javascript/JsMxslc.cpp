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
//     const mtlx = mx.compileSlxToMtlx('float x = add(1.0, 2.0);');
//     const slx  = mx.decompileMtlxToSlx(mtlx);
//     opts.delete();
//

#include <emscripten/bind.h>
#include <emscripten/val.h>

#include <cmath>
#include <exception>
#include <limits>
#include <set>

#include "compile.h"
#include "CompileOptions.h"
#include "decompile/decompile.h"
#include "preprocess/Macro.h"
#include "runtime/interface.h"
#include "runtime/variables/Variable.h"
#include "utils/load_mtlx.h"
#include "utils/token_utils.h"

#include "common.h"

namespace ems = emscripten;

namespace
{
    using mxslc::runtime::VarPtr;

    bool is_nullish(const ems::val& value)
    {
        return value.isUndefined() or value.isNull();
    }

    bool is_array(const ems::val& value)
    {
        return ems::val::global("Array").call<bool>("isArray", value);
    }

    string js_type_name(const ems::val& value)
    {
        if (value.isUndefined())
            return "undefined";
        if (value.isNull())
            return "null";
        return value.typeOf().as<string>();
    }

    vector<float> to_cpp_float_sequence(const ems::val& value)
    {
        if (not is_array(value))
            throw std::invalid_argument("Expected an array, got " + js_type_name(value));

        vector<float> result;
        const unsigned length = value["length"].as<unsigned>();
        for (unsigned i = 0; i < length; ++i)
        {
            const ems::val item = value[i];
            if (is_array(item))
            {
                vector<float> nested = to_cpp_float_sequence(item);
                result.insert(result.end(), nested.begin(), nested.end());
                continue;
            }

            if (item.typeOf().as<string>() != "number")
                throw std::invalid_argument("Expected a numeric array element, got " + js_type_name(item));

            result.push_back(item.as<float>());
        }

        return result;
    }

#define MARSHALL_SEQUENCE(n, T) \
    if (components.size() == n) \
        return mx::T{components.data(), components.data() + components.size()}

    mxslc::Primitive to_cpp_primitive(const ems::val& value)
    {
        if (is_nullish(value))
            return mxslc::Primitive{};

        const string type = value.typeOf().as<string>();
        if (type == "boolean")
            return value.as<bool>();
        if (type == "number")
        {
            const double number = value.as<double>();
            if (std::isfinite(number) and std::floor(number) == number and
                number >= static_cast<double>(std::numeric_limits<int>::min()) and
                number <= static_cast<double>(std::numeric_limits<int>::max()))
                return static_cast<int>(number);
            return static_cast<float>(number);
        }
        if (type == "string")
            return value.as<string>();
        if (is_array(value))
        {
            const vector<float> components = to_cpp_float_sequence(value);

            MARSHALL_SEQUENCE(2, Vector2);
            MARSHALL_SEQUENCE(3, Vector3);
            MARSHALL_SEQUENCE(4, Vector4);
            MARSHALL_SEQUENCE(9, Matrix33);
            MARSHALL_SEQUENCE(16, Matrix44);
        }

        throw std::invalid_argument("Cannot create a primitive from a JavaScript value of type " + type);
    }

#undef MARSHALL_SEQUENCE

    VarPtr to_cpp_variable(const ems::val& value)
    {
        if (is_array(value))
        {
            try
            {
                return mxslc::runtime::create_variable(to_cpp_primitive(value));
            }
            catch (const std::exception&)
            {
                vector<VarPtr> children;
                const unsigned length = value["length"].as<unsigned>();
                children.reserve(length);
                for (unsigned i = 0; i < length; ++i)
                    children.push_back(to_cpp_variable(value[i]));
                return mxslc::runtime::create_variable(children);
            }
        }

        return mxslc::runtime::create_variable(to_cpp_primitive(value));
    }

    mxslc::Macro to_cpp_macro(const ems::val& value)
    {
        if (value.typeOf().as<string>() == "string")
            return mxslc::Macro{value.as<string>()};

        if (is_array(value))
        {
            const unsigned length = value["length"].as<unsigned>();
            if (length == 1)
                return mxslc::Macro{value[0].as<string>()};
            if (length == 2)
                return mxslc::Macro{value[0].as<string>(), value[1].as<string>()};
        }

        throw std::invalid_argument("Cannot create a macro from a JavaScript value of type " + js_type_name(value));
    }

    ems::val to_js_array(const vector<float>& values)
    {
        ems::val result = ems::val::array();
        for (unsigned i = 0; i < values.size(); ++i)
            result.set(i, values[i]);
        return result;
    }

    ems::val to_js_primitive(const mxslc::Primitive& primitive)
    {
        return primitive.visit([](const auto& value) -> ems::val {
            IF_VISITED_TYPE_IS(std::monostate)
            {
                return ems::val::null();
            }
            IF_VISITED_TYPE_IS(fs::path)
            {
                return ems::val(value.string());
            }
            IF_VISITED_TYPE_IS(mx::Vector2)
            {
                return to_js_array({value[0], value[1]});
            }
            IF_VISITED_TYPE_IS(mx::Vector3)
            {
                return to_js_array({value[0], value[1], value[2]});
            }
            IF_VISITED_TYPE_IS(mx::Vector4)
            {
                return to_js_array({value[0], value[1], value[2], value[3]});
            }
            IF_VISITED_TYPE_IS(mx::Color3)
            {
                return to_js_array({value[0], value[1], value[2]});
            }
            IF_VISITED_TYPE_IS(mx::Color4)
            {
                return to_js_array({value[0], value[1], value[2], value[3]});
            }
            IF_VISITED_TYPE_IS(mx::Matrix33)
            {
                ems::val rows = ems::val::array();
                for (unsigned row = 0; row < 3; ++row)
                    rows.set(row, to_js_array({value[row][0], value[row][1], value[row][2]}));
                return rows;
            }
            IF_VISITED_TYPE_IS(mx::Matrix44)
            {
                ems::val rows = ems::val::array();
                for (unsigned row = 0; row < 4; ++row)
                    rows.set(row, to_js_array({value[row][0], value[row][1], value[row][2], value[row][3]}));
                return rows;
            }
            return ems::val(value);
        });
    }

    ems::val to_js_variable(const VarPtr& var)
    {
        if (var->is_compile_time())
        {
            if (var->has_value())
                return to_js_primitive(var->compile_time_value());

            ems::val children = ems::val::array();
            const vector<VarPtr>& child_values = var->children();
            for (unsigned i = 0; i < child_values.size(); ++i)
                children.set(i, to_js_variable(child_values[i]));
            return children;
        }

        return ems::val::null();
    }

    mxslc::CompileOptions compile_options_from_js(const ems::val& value)
    {
        if (is_nullish(value))
            return {};
        return value.as<mxslc::CompileOptions>();
    }

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
                                    const ems::val& opts_value)
    {
        try
        {
            return mxslc::compile_to_string(source, compile_options_from_js(opts_value));
        }
        catch (const std::exception& e)
        {
            rethrow_as_js_error(e, "CompileError");
        }
    }

    // Compile an SLX source file in the WASM filesystem to a MaterialX (MTLX)
    // XML string using the given compile options.
    std::string compile_slx_file_to_mtlx(const std::string& src_path,
                                         const ems::val& opts_value)
    {
        try
        {
            return mxslc::compile_to_string(fs::path{src_path}, compile_options_from_js(opts_value));
        }
        catch (const std::exception& e)
        {
            rethrow_as_js_error(e, "CompileError");
        }
    }

    // Compile an SLX source file in the WASM filesystem to a MaterialX file in
    // the WASM filesystem. Returns the output path.
    std::string compile_slx_file_to_mtlx_file(const std::string& src_path,
                                              const ems::val& opts_value)
    {
        try
        {
            return mxslc::compile_to_file(fs::path{src_path}, compile_options_from_js(opts_value)).string();
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
        .property("outputFile",
            [](const mxslc::CompileOptions& opts) -> ems::val {
                return opts.output_file ? ems::val(opts.output_file->string()) : ems::val::null();
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                opts.output_file = is_nullish(value) ? std::nullopt : optional<fs::path>{value.as<string>()};
            }
        )
        .property("version", &mxslc::CompileOptions::version)
        .property("macros",
            [](const mxslc::CompileOptions& opts) {
                ems::val result = ems::val::array();
                vector<string> names;
                names.reserve(opts.macros().size());
                for (const auto& [name, macro] : opts.macros())
                    names.push_back(name);
                std::sort(names.begin(), names.end());

                for (unsigned i = 0; i < names.size(); ++i)
                {
                    const mxslc::Macro& macro = opts.get_macro(names[i]);
                    if (macro.body().empty())
                    {
                        result.set(i, ems::val(macro.name()));
                    }
                    else
                    {
                        ems::val entry = ems::val::array();
                        entry.set(0, macro.name());
                        entry.set(1, mxslc::token_utils::join_tokens(macro.body()));
                        result.set(i, entry);
                    }
                }
                return result;
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                if (not is_array(value))
                    throw std::invalid_argument("CompileOptions.macros must be an array");

                const unsigned length = value["length"].as<unsigned>();
                vector<mxslc::Macro> macros;
                macros.reserve(length);
                for (unsigned i = 0; i < length; ++i)
                    macros.push_back(to_cpp_macro(value[i]));

                opts.clear_macros();
                for (mxslc::Macro& macro : macros)
                    opts.add_macro(std::move(macro));
            }
        )
        .property("searchDirectories",
            [](const mxslc::CompileOptions& opts) {
                ems::val result = ems::val::array();
                const vector<fs::path> dirs = opts.search_directories();
                for (unsigned i = 0; i < dirs.size(); ++i)
                    result.set(i, dirs[i].string());
                return result;
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                if (not is_array(value))
                    throw std::invalid_argument("CompileOptions.searchDirectories must be an array");

                const unsigned length = value["length"].as<unsigned>();
                vector<fs::path> dirs;
                dirs.reserve(length);
                for (unsigned i = 0; i < length; ++i)
                    dirs.emplace_back(value[i].as<string>());

                opts.clear_search_directories();
                for (fs::path& dir : dirs)
                    opts.add_search_directory(std::move(dir));
            }
        )
        .property("includes",
            [](const mxslc::CompileOptions& opts) {
                ems::val result = ems::val::array();
                for (unsigned i = 0; i < opts.includes.size(); ++i)
                    result.set(i, opts.includes[i].string());
                return result;
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                if (not is_array(value))
                    throw std::invalid_argument("CompileOptions.includes must be an array");

                const unsigned length = value["length"].as<unsigned>();
                vector<fs::path> includes;
                includes.reserve(length);
                for (unsigned i = 0; i < length; ++i)
                    includes.emplace_back(value[i].as<string>());

                opts.includes = std::move(includes);
            }
        )
        .property("libraries",
            [](const mxslc::CompileOptions& opts) {
                ems::val result = ems::val::array();
                for (unsigned i = 0; i < opts.libraries.size(); ++i)
                    result.set(i, opts.libraries[i].string());
                return result;
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                if (not is_array(value))
                    throw std::invalid_argument("CompileOptions.libraries must be an array");

                const unsigned length = value["length"].as<unsigned>();
                vector<fs::path> libraries;
                libraries.reserve(length);
                for (unsigned i = 0; i < length; ++i)
                    libraries.emplace_back(value[i].as<string>());

                opts.libraries = std::move(libraries);
            }
        )
        .property("globals",
            [](const mxslc::CompileOptions& opts) {
                ems::val result = ems::val::object();
                for (const auto& [name, value] : opts.globals())
                    result.set(name, to_js_variable(value));
                return result;
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                if (is_nullish(value) or is_array(value) or value.typeOf().as<string>() != "object")
                    throw std::invalid_argument("CompileOptions.globals must be an object");

                const ems::val names = ems::val::global("Object").call<ems::val>("keys", value);
                const unsigned length = names["length"].as<unsigned>();
                unordered_map<string, VarPtr> globals;
                globals.reserve(length);
                for (unsigned i = 0; i < length; ++i)
                {
                    const string name = names[i].as<string>();
                    globals.emplace(name, to_cpp_variable(value[name]));
                }

                opts.set_globals(std::move(globals));
            }
        )
        .property("errorOnMissingGlobals", &mxslc::CompileOptions::error_on_missing_globals)
        .property("errorOnUnusedGlobals", &mxslc::CompileOptions::error_on_unused_globals)
        .property("funcName",
            [](const mxslc::CompileOptions& opts) -> ems::val {
                return opts.func_name ? ems::val(*opts.func_name) : ems::val::null();
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                opts.func_name = is_nullish(value) ? std::nullopt : optional<string>{value.as<string>()};
            }
        )
        .property("funcArgs",
            [](const mxslc::CompileOptions& opts) {
                ems::val result = ems::val::array();
                const vector<VarPtr>& args = opts.entry_function_arguments();
                for (unsigned i = 0; i < args.size(); ++i)
                    result.set(i, to_js_variable(args[i]));
                return result;
            },
            [](mxslc::CompileOptions& opts, const ems::val& value) {
                if (not is_array(value))
                    throw std::invalid_argument("CompileOptions.funcArgs must be an array");

                const unsigned length = value["length"].as<unsigned>();
                vector<VarPtr> args;
                args.reserve(length);
                for (unsigned i = 0; i < length; ++i)
                    args.push_back(to_cpp_variable(value[i]));

                opts.set_entry_function_arguments(std::move(args));
            }
        )
        .property("reduceGraph", &mxslc::CompileOptions::reduce_graph)
        .property("validateGraph", &mxslc::CompileOptions::validate_graph);

    ems::function("compileSlxToMtlx", &compile_slx_to_mtlx);
    ems::function("compileSlxFileToMtlx", &compile_slx_file_to_mtlx);
    ems::function("compileSlxFileToMtlxFile", &compile_slx_file_to_mtlx_file);
    ems::function("decompileMtlxToSlx", &decompile_mtlx_to_slx);

    ems::register_vector<std::string>("StringVector");
    ems::function("getMtlxDefinitionNames", &get_mtlx_definition_names);
}
