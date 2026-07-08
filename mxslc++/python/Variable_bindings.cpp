//
// Created by jaket on 24/06/2026.
//

#include "utils.h"

void bind_variable(py::module_& m)
{
    py::class_<mxslc::interface::Variable, mxslc::interface::VariablePtr>(m, "Variable")
        .def(py::init([](string type, string name, vector<mxslc::interface::VariablePtr> children) {
            return std::make_shared<mxslc::interface::Variable>(std::move(type), std::move(name), std::move(children));
        }), py::arg("type"), py::arg("name"), py::arg("children"))

        .def(py::init([](string type, string name, const py::list& children) {
            vector<mxslc::primitive_t> prims;
            for (py::handle item : children) prims.push_back(utils::marshall_primitive(item));
            return std::make_shared<mxslc::interface::Variable>(std::move(type), std::move(name), std::move(prims));
        }), py::arg("type"), py::arg("name"), py::arg("primitive_children"))

        .def(py::init([](string type, string name, py::handle value) {
            return std::make_shared<mxslc::interface::Variable>(std::move(type), std::move(name), utils::marshall_primitive(value));
        }), py::arg("type"), py::arg("name"), py::arg("value"))

        .def(py::init([](string name, vector<mxslc::interface::VariablePtr> children) {
            return std::make_shared<mxslc::interface::Variable>(std::move(name), std::move(children));
        }), py::arg("name"), py::arg("children"))

        .def(py::init([](string name, const py::list& children) {
            vector<mxslc::primitive_t> prims;
            for (py::handle item : children) prims.push_back(utils::marshall_primitive(item));
            return std::make_shared<mxslc::interface::Variable>(std::move(name), std::move(prims));
        }), py::arg("name"), py::arg("primitive_children"))

        .def(py::init([](string name, const py::handle value) {
            return std::make_shared<mxslc::interface::Variable>(std::move(name), utils::marshall_primitive(value));
        }), py::arg("name"), py::arg("value"))

        .def(py::init([](vector<mxslc::interface::VariablePtr> children) {
            return std::make_shared<mxslc::interface::Variable>(std::move(children));
        }), py::arg("children"))

        .def(py::init([](const py::list& children) {
            vector<mxslc::primitive_t> prims;
            for (py::handle item : children) prims.push_back(utils::marshall_primitive(item));
            return std::make_shared<mxslc::interface::Variable>(std::move(prims));
        }), py::arg("primitive_children"))

        .def(py::init([](const py::handle value) {
            return std::make_shared<mxslc::interface::Variable>(utils::marshall_primitive(value));
        }), py::arg("value"));
}
