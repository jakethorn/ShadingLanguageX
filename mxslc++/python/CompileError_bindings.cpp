//
// Created by jaket on 23/11/2026.
//

#include "CompileError_bindings.h"

#include "errors/CompileError.h"

void bind_compile_error(py::module_& m)
{
    py::class_<Diagnostic>(m, "Diagnostic")
        .def_property_readonly("file", [](const Diagnostic& diagnostic) { return diagnostic.location.path; })
        .def_property_readonly("line", [](const Diagnostic& diagnostic) { return diagnostic.location.line; })
        .def_readonly("message", &Diagnostic::message)
        .def("__str__", &Diagnostic::to_string)
        .def("__repr__", [](const Diagnostic& diagnostic)
        {
            return "Diagnostic('" + diagnostic.to_string() + "')";
        });

    PYBIND11_CONSTINIT static py::gil_safe_call_once_and_store<py::object> compile_error;
    compile_error.call_once_and_store_result([&m]
    {
        return py::exception<CompileError>(m, "CompileError", PyExc_RuntimeError);
    });

    py::register_exception_translator([](std::exception_ptr p)
    {
        try
        {
            if (p)
                std::rethrow_exception(p);
        }
        catch (const CompileError& e)
        {
            const py::object& error_type = compile_error.get_stored();
            py::object error = error_type(e.what());
            error.attr("diagnostics") = e.diagnostics();
            py::set_error(error_type, error);
        }
    });
}
