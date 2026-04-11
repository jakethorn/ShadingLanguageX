//
// Created by jaket on 11/04/2026.
//

#include "NodeValue.h"

#include "CompileError.h"

ValuePtr NodeValue::subvalue(const size_t i) const
{
    size_t j = 0;
    for (const mx::OutputPtr& output : node_->getOutputs())
    {
        if (i == j++)
            return std::make_shared<OutputValue>(output, type_->field_type(i));
    }

    throw CompileError{"Trying to access multiple outputs from a node (" + node_->getName() + ") that only has one output"};
}
