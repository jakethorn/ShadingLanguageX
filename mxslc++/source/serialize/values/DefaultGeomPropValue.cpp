//
// Created by jaket on 15/09/2026.
//

#include "serialize/values/DefaultGeomPropValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize::values
{
    DefaultGeomPropValue::DefaultGeomPropValue(string name) : Value{get_type(name)}, name_{std::move(name)} { }

    bool DefaultGeomPropValue::equals(const ValuePtr& other) const
    {
        if (const DefaultGeomPropValuePtr other_basic = cast_value<DefaultGeomPropValue>(other))
            return name_ == other_basic->name_;
        return false;
    }

    void DefaultGeomPropValue::set_as_node_input(const mx::InputPtr& input) const
    {
        throw CompileError{"defaultgeomprop attributes can only be used on NodeDef inputs"};
    }

    void DefaultGeomPropValue::set_as_node_graph_output(const mx::NodeGraphPtr& node_graph, const string& output_name) const
    {
        throw CompileError{"defaultgeomprop attributes can only be used on NodeDef inputs"};
    }

    void DefaultGeomPropValue::set_as_node_def_input(const mx::NodeDefPtr& node_def, const string& input_name) const
    {
        const mx::InputPtr input = node_def->addInput(input_name, type_->name());
        input->setDefaultGeomPropString(name_);
    }

    void DefaultGeomPropValue::set_as_node_graph_input(const mx::NodeGraphPtr& node_graph, const string& input_name) const
    {
        const mx::InputPtr input = node_graph->addInput(input_name, type_->name());
        input->setDefaultGeomPropString(name_);
    }

    string DefaultGeomPropValue::to_string() const
    {
        return name_;
    }

    TypePtr DefaultGeomPropValue::get_type(const string& name)
    {
        const vector<string> vec3_types{
            "Pobject",
            "Nobject",
            "Tobject",
            "Bobject",
            "Pworld",
            "Nworld",
            "Tworld",
            "Bworld",
        };

        const vector<string> vec2_types{
            "UV0",
        };

        if (container_utils::contains(vec3_types, name))
            return Type::Vec3;
        if (container_utils::contains(vec2_types, name))
            return Type::Vec2;
        throw CompileError{"Unknown defaultgeomprop name: '" + name + "'"};
    }
}
