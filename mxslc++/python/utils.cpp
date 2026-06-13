//
// Created by jaket on 12/06/2026.
//

#include "utils.h"

vector<float> utils::marshall_float_sequence(const py::handle& py_obj)
{
    if (py::isinstance<py::str>(py_obj))
        throw py::type_error("Error when marshalling float sequence");

    vector<float> result;
    for (auto element : py_obj.cast<py::sequence>())
    {
        if (py::isinstance<py::sequence>(element))
        {
            for (float sub_element : marshall_float_sequence(element))
                result.push_back(sub_element);
        }
        else if (py::isinstance<py::float_>(element) or py::isinstance<py::int_>(element))
        {
            result.push_back(element.cast<float>());
        }
        else
        {
            throw py::type_error("Error when marshalling float sequence");
        }
    }

    return result;
}

#define MARSHALL_VECTOR(T) \
    if (class_name == #T and cs.size() == mx::T::numElements()) { \
        return mx::T{cs.data(), cs.data() + cs.size()}; \
    }

#define MARSHALL_MATRIX(T) \
    if (class_name == #T and cs.size() == mx::T::numRows() * mx::T::numColumns()) { \
        return mx::T{cs.data(), cs.data() + cs.size()}; \
    }

#define MARSHALL_SEQUENCE(n, T) \
    if (cs.size() == n) { \
        return mx::T{cs.data(), cs.data() + cs.size()}; \
    }

mxslc::primitive_t utils::marshall_primitive(const py::handle& py_obj)
{
    if (py::isinstance<py::bool_>(py_obj))
        return py_obj.cast<bool>();
    if (py::isinstance<py::int_>(py_obj))
        return py_obj.cast<int>();
    if (py::isinstance<py::float_>(py_obj))
        return py_obj.cast<float>();
    if (py::isinstance<py::str>(py_obj))
        return py_obj.cast<string>();
    if (py::hasattr(py_obj, "__fspath__"))
        return py_obj.cast<std::filesystem::path>();

    if (py::isinstance<py::sequence>(py_obj))
    {
        string class_name;
        if (py::hasattr(py_obj, "__class__"))
            class_name = py::str(py_obj.attr("__class__").attr("__name__"));

        const vector<float> cs = marshall_float_sequence(py_obj);

        MARSHALL_VECTOR(Vector2)
        MARSHALL_VECTOR(Vector3)
        MARSHALL_VECTOR(Vector4)
        MARSHALL_VECTOR(Color3)
        MARSHALL_VECTOR(Color4)
        MARSHALL_MATRIX(Matrix33)
        MARSHALL_MATRIX(Matrix44)

        MARSHALL_SEQUENCE(2, Vector2)
        MARSHALL_SEQUENCE(3, Vector3)
        MARSHALL_SEQUENCE(4, Vector4)
        MARSHALL_SEQUENCE(9, Matrix33)
        MARSHALL_SEQUENCE(16, Matrix44)
    }

    throw py::type_error("Error when marshalling primitive type");
}

#undef MARSHALL_VECTOR
#undef MARSHALL_MATRIX
#undef MARSHALL_SEQUENCE
