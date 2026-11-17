//
// Created by jaket on 15/09/2026.
//

#include "serialize/values/DefaultGeomPropValue.h"

#include "runtime/Type.h"
#include "utils/mtlx_utils.h"
#include "serialize/values/interface.h"

namespace mxslc::serialize::values
{
    DefaultGeomPropValue::DefaultGeomPropValue(mx::GeomPropDefPtr geomprop)
        : Value{Type::of(geomprop)}, geomprop_{std::move(geomprop)} { }

    bool DefaultGeomPropValue::equals(const ValuePtr& other) const
    {
        if (const DefaultGeomPropValuePtr other_basic = cast_value<DefaultGeomPropValue>(other))
            return geomprop_ == other_basic->geomprop_;
        return false;
    }

    void DefaultGeomPropValue::set_as_node_input(const mx::InputPtr& input) const
    {
        throw CompileError{"defaultgeomprop attributes can only be used on NodeDef/NodeGraph inputs"};
    }

    void DefaultGeomPropValue::set_as_node_def_input(const mx::InputPtr& input) const
    {
        input->setDefaultGeomPropString(geomprop_->getName());
    }

    void DefaultGeomPropValue::set_as_node_graph_output(const mx::OutputPtr& output) const
    {
        throw CompileError{"defaultgeomprop attributes can only be used on NodeDef/NodeGraph inputs"};
    }

    void DefaultGeomPropValue::set_as_node_graph_input(const mx::InputPtr& input) const
    {
        input->setDefaultGeomPropString(geomprop_->getName());
    }

    string DefaultGeomPropValue::to_string() const
    {
        return geomprop_->getName();
    }
}
