//
// Created by jaket on 24/06/2026.
//

#include "CompileOptions_bindings.h"

#include "utils.h"

void bind_compile_options(py::module_& m)
{
    py::class_<mxslc::CompileOptions>(m, "CompileOptions")
        .def(
            py::init([](
                optional<fs::path> output_file,
                string version,
                optional<string> func_name,
                const py::list& func_args,
                const py::list& globals,
                const bool error_on_missing_globals,
                const bool error_on_unused_globals,
                const bool reduce_graph)
            {
                auto opts = std::make_unique<mxslc::CompileOptions>();
                opts->output_file = std::move(output_file);
                opts->version = std::move(version);
                opts->func_name = std::move(func_name);
                opts->reduce_graph = reduce_graph;
                opts->error_on_missing_globals = error_on_missing_globals;
                opts->error_on_unused_globals = error_on_unused_globals;

                for (py::handle py_obj : func_args)
                    opts->func_args.push_back(utils::marshall_primitive(py_obj));

                for (py::handle py_obj : globals)
                {
                    auto var_ptr = py_obj.cast<mxslc::VariablePtr>();
                    opts->globals.push_back(*var_ptr);
                }

                return opts;
            }),
            py::arg("output_file") = std::nullopt,
            py::arg("version") = mxslc::CompileOptions{}.version,
            py::arg("func_name") = std::nullopt,
            py::arg("func_args") = py::list(),
            py::arg("globals") = py::list(),
            py::arg("error_on_missing_globals") = mxslc::CompileOptions{}.error_on_missing_globals,
            py::arg("error_on_unused_globals") = mxslc::CompileOptions{}.error_on_unused_globals,
            py::arg("reduce_graph") = mxslc::CompileOptions{}.reduce_graph
        )
        .def_readwrite("output_file", &mxslc::CompileOptions::output_file)
        .def_readwrite("version", &mxslc::CompileOptions::version)
        .def_readwrite("error_on_missing_globals", &mxslc::CompileOptions::error_on_missing_globals)
        .def_readwrite("error_on_unused_globals", &mxslc::CompileOptions::error_on_unused_globals)
        .def_readwrite("reduce_graph", &mxslc::CompileOptions::reduce_graph)
        .def_readwrite("func_name", &mxslc::CompileOptions::func_name)
        .def_property(
            "func_args",
            nullptr, // getter
            [](mxslc::CompileOptions& opts, const py::list& py_args) {
                opts.func_args.clear();
                for (py::handle py_obj : py_args)
                    opts.func_args.push_back(utils::marshall_primitive(py_obj));
            }
        ).def_property(
            "globals",
            nullptr, // getter
            [](mxslc::CompileOptions& opts, const py::list& py_globals) {
                opts.globals.clear();
                for (py::handle py_obj : py_globals) {
                    auto var_ptr = py_obj.cast<mxslc::VariablePtr>();
                    opts.globals.push_back(*var_ptr);
                }
            }
        );
}
