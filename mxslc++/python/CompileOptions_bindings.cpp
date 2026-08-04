//
// Created by jaket on 24/06/2026.
//

#include "CompileOptions_bindings.h"

#include "utils.h"

#include "CompileOptions.h"
#include "runtime/variables/Variable.h"

void bind_compile_options(py::module_& m)
{
    py::class_<CompileOptions>(m, "CompileOptions")
        .def(
            py::init([](
                const optional<fs::path>& output_file,
                const string& version,
                const py::list& macros,
                const vector<fs::path>& search_directories,
                const vector<fs::path>& includes,
                const vector<fs::path>& libraries,
                const unordered_map<string, py::object>& globals,
                const bool error_on_missing_globals,
                const bool error_on_unused_globals,
                const optional<string>& func_name,
                const py::list& func_args,
                const bool reduce_graph)
            {
                auto opts = std::make_unique<CompileOptions>();
                opts->output_file = output_file;
                opts->version = version;

                for (const py::handle& macro : macros)
                    opts->add_macro(utils::to_cpp_macro(macro));

                for (const fs::path& dir : search_directories)
                    opts->add_search_directory(dir);

                opts->includes = includes;
                opts->libraries = libraries;

                for (const auto& [name, value] : globals)
                    opts->add_global(name, utils::to_cpp_variable(value));

                opts->error_on_missing_globals = error_on_missing_globals;
                opts->error_on_unused_globals = error_on_unused_globals;
                opts->func_name = func_name;

                for (const py::handle& arg : func_args)
                    opts->add_entry_function_argument(utils::to_cpp_variable(arg));

                opts->reduce_graph = reduce_graph;
                return opts;
            }),
            py::arg("output_file") = std::nullopt,
            py::arg("version") = CompileOptions{}.version,
            py::arg("macros") = py::list(),
            py::arg("search_directories") = py::list(),
            py::arg("includes") = py::list(),
            py::arg("libraries") = py::list(),
            py::arg("globals") = py::dict(),
            py::arg("error_on_missing_globals") = CompileOptions{}.error_on_missing_globals,
            py::arg("error_on_unused_globals") = CompileOptions{}.error_on_unused_globals,
            py::arg("func_name") = std::nullopt,
            py::arg("func_args") = py::list(),
            py::arg("reduce_graph") = CompileOptions{}.reduce_graph
        )
        .def_readwrite("output_file", &CompileOptions::output_file)
        .def_readwrite("version", &CompileOptions::version)
        .def_property("macros",
            [](const CompileOptions& opts) {
                return opts.macros();
            },
            [](CompileOptions& opts, const py::list& macros) {
                opts.clear_macros();
                for (const py::handle& macro : macros)
                    opts.add_macro(utils::to_cpp_macro(macro));
            }
        )
        .def_property("search_directories",
            [](const CompileOptions& opts) {
                return opts.search_directories();
            },
            [](CompileOptions& opts, const vector<fs::path>& search_directories) {
                opts.clear_search_directories();
                for (const fs::path& dir : search_directories)
                    opts.add_search_directory(dir);
            }
        )
        .def_readwrite("includes", &CompileOptions::includes)
        .def_readwrite("libraries", &CompileOptions::libraries)
        .def_property("globals",
            [](const CompileOptions& opts) {
                return opts.globals();
            },
            [](CompileOptions& opts, const unordered_map<string, py::object>& globals) {
                opts.clear_globals();
                for (const auto& [name, value] : globals)
                    opts.add_global(name, utils::to_cpp_variable(value));
            }
        )
        .def_readwrite("error_on_missing_globals", &CompileOptions::error_on_missing_globals)
        .def_readwrite("error_on_unused_globals", &CompileOptions::error_on_unused_globals)
        .def_readwrite("func_name", &CompileOptions::func_name)
        .def_property("func_args",
            [](const CompileOptions& opts) {
                return opts.entry_function_arguments();
            },
            [](CompileOptions& opts, const py::list& func_args) {
                opts.clear_entry_function_arguments();
                for (const py::handle& arg : func_args)
                    opts.add_entry_function_argument(utils::to_cpp_variable(arg));
            }
        )
        .def_readwrite("reduce_graph", &CompileOptions::reduce_graph);
}
