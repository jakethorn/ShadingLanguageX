//
// Created by jaket on 12/06/2026.
//

#include "utils.h"

#include "Primitive.h"
#include "runtime/interface.h"
#include "runtime/variables/Variable.h"

namespace utils
{
    string get_type_name(const py::handle& py_obj)
    {
        return py::type::of(py_obj).attr("__name__").cast<string>();
    }

    vector<float> to_cpp_float_sequence(const py::handle& py_obj)
    {
        if (not py::isinstance<py::sequence>(py_obj) or py::isinstance<py::str>(py_obj))
            throw py::type_error("Cannot create a vector<float> from an object of type " + get_type_name(py_obj));

        vector<float> result;
        for (const py::handle& item : py_obj.cast<py::sequence>())
        {
            if (py::isinstance<py::sequence>(item))
            {
                for (float sub_item : to_cpp_float_sequence(item))
                    result.push_back(sub_item);
            }
            else if (py::isinstance<py::float_>(item) or py::isinstance<py::int_>(item))
            {
                result.push_back(item.cast<float>());
            }
            else
            {
                throw py::type_error("Cannot create a vector<float> with an element of type " + get_type_name(item));
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

    Primitive to_cpp_primitive(const py::handle& py_obj)
    {
        if (py_obj.is_none())
            return Primitive{};
        if (py::isinstance<py::bool_>(py_obj))
            return py_obj.cast<bool>();
        if (py::isinstance<py::int_>(py_obj))
            return py_obj.cast<int>();
        if (py::isinstance<py::float_>(py_obj))
            return py_obj.cast<float>();
        if (py::isinstance<py::str>(py_obj))
            return py_obj.cast<string>();
        if (py::hasattr(py_obj, "__fspath__"))
            return py_obj.cast<fs::path>();

        if (py::isinstance<py::sequence>(py_obj))
        {
            string class_name;
            if (py::hasattr(py_obj, "__class__"))
                class_name = py::str(py_obj.attr("__class__").attr("__name__"));

            const vector<float> cs = to_cpp_float_sequence(py_obj);

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

        throw py::type_error("Cannot create a Primitive from an object of type " + get_type_name(py_obj));
    }

#undef MARSHALL_VECTOR
#undef MARSHALL_MATRIX
#undef MARSHALL_SEQUENCE

    VarPtr to_cpp_variable(const py::handle& py_obj)
    {
        if (py::isinstance<Variable>(py_obj))
            return py_obj.cast<VarPtr>();
        if (is_primitive_type(py_obj))
        {
            Primitive value = to_cpp_primitive(py_obj);
            return create_variable(std::move(value));
        }
        if (py::isinstance<py::sequence>(py_obj))
        {
            vector<VarPtr> children;
            for (const py::handle& item : py_obj.cast<py::sequence>())
            {
                VarPtr child = to_cpp_variable(item);
                children.push_back(std::move(child));
            }
            return create_variable(children);
        }

        throw py::type_error("Cannot create a Variable from an object of type " + get_type_name(py_obj));
    }

    Macro to_cpp_macro(const py::handle& py_obj)
    {
        if (py::isinstance<Macro>(py_obj))
            return py_obj.cast<Macro>();
        if (py::isinstance<py::str>(py_obj))
            return Macro{py_obj.cast<string>()};
        if (py::isinstance<py::sequence>(py_obj))
        {
            py::sequence parts = py_obj.cast<py::sequence>();
            if (parts.size() == 1)
                return Macro{parts[0].cast<string>()};
            if (parts.size() == 2)
                return Macro{parts[0].cast<string>(), parts[1].cast<string>()};
        }
        throw py::type_error("Cannot create a Macro from an object of type " + get_type_name(py_obj));
    }

    py::object to_py_primitive(const Primitive& primitive)
    {
        return primitive.visit([](const auto& v) -> py::object {
            IF_VISITED_TYPE_IS(std::monostate)
            {
                return py::none();
            }
            IF_VISITED_TYPE_IS(mx::Vector2)
            {
                py::module_ mx = py::module_::import("MaterialX");
                return mx.attr("Vector2")(v[0], v[1]);
            }
            IF_VISITED_TYPE_IS(mx::Vector3)
            {
                py::module_ mx = py::module_::import("MaterialX");
                return mx.attr("Vector3")(v[0], v[1], v[2]);
            }
            IF_VISITED_TYPE_IS(mx::Vector4)
            {
                py::module_ mx = py::module_::import("MaterialX");
                return mx.attr("Vector4")(v[0], v[1], v[2], v[3]);
            }
            IF_VISITED_TYPE_IS(mx::Color3)
            {
                py::module_ mx = py::module_::import("MaterialX");
                return mx.attr("Color3")(v[0], v[1], v[2]);
            }
            IF_VISITED_TYPE_IS(mx::Color4)
            {
                py::module_ mx = py::module_::import("MaterialX");
                return mx.attr("Color4")(v[0], v[1], v[2], v[3]);
            }
            IF_VISITED_TYPE_IS(mx::Matrix33)
            {
                py::module_ mx = py::module_::import("MaterialX");
                return mx.attr("Matrix33")(
                    v[0][0], v[0][1], v[0][2],
                    v[1][0], v[1][1], v[1][2],
                    v[2][0], v[2][1], v[2][2]
                );
            }
            IF_VISITED_TYPE_IS(mx::Matrix44)
            {
                py::module_ mx = py::module_::import("MaterialX");
                return mx.attr("Matrix44")(
                    v[0][0], v[0][1], v[0][2], v[0][3],
                    v[1][0], v[1][1], v[1][2], v[1][3],
                    v[2][0], v[2][1], v[2][2], v[2][3],
                    v[3][0], v[3][1], v[3][2], v[3][3]
                );
            }
            return py::cast(v);
        });
    }

    py::object to_py_variable(const VarPtr& var)
    {
        if (var->is_compile_time())
        {
            if (var->has_value())
            {
                return to_py_primitive(var->compile_time_value());
            }
            else
            {
                py::list children;
                for (const VarPtr& child : var->children())
                    children.append(to_py_variable(child));
                return children;
            }
        }
        else
        {
            return py::none();
        }
    }

    bool is_primitive_type(const py::handle& py_obj)
    {
        if (py::isinstance<py::bool_>(py_obj))
            return true;
        if (py::isinstance<py::int_>(py_obj))
            return true;
        if (py::isinstance<py::float_>(py_obj))
            return true;
        if (py::isinstance<py::str>(py_obj))
            return true;
        if (py::hasattr(py_obj, "__fspath__"))
            return true;

        try
        {
            py::module_ mx = py::module_::import("MaterialX");
#define IS_TYPE(type) py::isinstance(py_obj, mx.attr(#type))
            return IS_TYPE(Vector2) or IS_TYPE(Vector3) or IS_TYPE(Vector4) or IS_TYPE(Color3) or IS_TYPE(Color4) or IS_TYPE(Matrix33) or IS_TYPE(Matrix44);
#undef IS_TYPE
        }
        catch (const py::error_already_set&)
        {
            return false;
        }
    }


}
