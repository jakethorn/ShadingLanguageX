//
// Created by jaket on 04/06/2026.
//

#include "common.h"
#include "utils.h"

PYBIND11_MODULE(_mxslc, m)
{
    m.doc() = "Python bindings for mxslc";

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

    py::class_<mxslc::Variable, mxslc::VariablePtr>(m, "Variable")
        .def(py::init([](string type, string name, vector<mxslc::VariablePtr> children) {
            return std::make_shared<mxslc::Variable>(std::move(type), std::move(name), std::move(children));
        }), py::arg("type"), py::arg("name"), py::arg("children"))

        .def(py::init([](string type, string name, const py::list& children) {
            vector<mxslc::primitive_t> prims;
            for (py::handle item : children) prims.push_back(utils::marshall_primitive(item));
            return std::make_shared<mxslc::Variable>(std::move(type), std::move(name), std::move(prims));
        }), py::arg("type"), py::arg("name"), py::arg("primitive_children"))
        
        .def(py::init([](string type, string name, py::handle value) {
            return std::make_shared<mxslc::Variable>(std::move(type), std::move(name), utils::marshall_primitive(value));
        }), py::arg("type"), py::arg("name"), py::arg("value"))

        .def(py::init([](string name, vector<mxslc::VariablePtr> children) {
            return std::make_shared<mxslc::Variable>(std::move(name), std::move(children));
        }), py::arg("name"), py::arg("children"))
        
        .def(py::init([](string name, const py::list& children) {
            vector<mxslc::primitive_t> prims;
            for (py::handle item : children) prims.push_back(utils::marshall_primitive(item));
            return std::make_shared<mxslc::Variable>(std::move(name), std::move(prims));
        }), py::arg("name"), py::arg("primitive_children"))
        
        .def(py::init([](string name, const py::handle value) {
            return std::make_shared<mxslc::Variable>(std::move(name), utils::marshall_primitive(value));
        }), py::arg("name"), py::arg("value"))

        .def(py::init([](vector<mxslc::VariablePtr> children) {
            return std::make_shared<mxslc::Variable>(std::move(children));
        }), py::arg("children"))
        
        .def(py::init([](const py::list& children) {
            vector<mxslc::primitive_t> prims;
            for (py::handle item : children) prims.push_back(utils::marshall_primitive(item));
            return std::make_shared<mxslc::Variable>(std::move(prims));
        }), py::arg("primitive_children"))
        
        .def(py::init([](const py::handle value) {
            return std::make_shared<mxslc::Variable>(utils::marshall_primitive(value));
        }), py::arg("value"));
    
    m.def(
        "compile_file_to_string",
        py::overload_cast<const fs::path&>(&mxslc::compile_to_string),
        py::arg("src_path")
    );

    m.def(
        "compile_file_to_string",
        py::overload_cast<const fs::path&, const mxslc::CompileOptions&>(&mxslc::compile_to_string),
        py::arg("src_path"),
        py::arg("opts")
    );

    m.def(
        "compile_string_to_string",
        py::overload_cast<const string&>(&mxslc::compile_to_string),
        py::arg("source")
    );

    m.def(
        "compile_string_to_string",
        py::overload_cast<const string&, const mxslc::CompileOptions&>(&mxslc::compile_to_string),
        py::arg("source"),
        py::arg("opts")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const fs::path&>(&mxslc::compile_to_file),
        py::arg("src_path")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const fs::path&, const mxslc::CompileOptions&>(&mxslc::compile_to_file),
        py::arg("src_path"),
        py::arg("opts")
    );

    m.def(
        "compile_file_to_file",
        py::overload_cast<const fs::path&, const fs::path&>(&mxslc::compile_to_file),
        py::arg("src_path"),
        py::arg("dst_path")
    );

    m.def(
        "compile_string_to_file",
        py::overload_cast<const string&, const mxslc::CompileOptions&>(&mxslc::compile_to_file),
        py::arg("source"),
        py::arg("opts")
    );

    m.def(
        "compile_string_to_file",
        py::overload_cast<const string&, const fs::path&>(&mxslc::compile_to_file),
        py::arg("source"),
        py::arg("dst_path")
    );
}
