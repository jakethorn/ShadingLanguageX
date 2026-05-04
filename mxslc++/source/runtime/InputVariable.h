//
// Created by jaket on 03/05/2026.
//

#ifndef MXSLC_INPUTVARIABLE_H
#define MXSLC_INPUTVARIABLE_H

#include "utils/common.h"
#include "Variable.h"
#include "values/ValueFactory.h"

class InputVariable final : public Variable
{
public:
    explicit InputVariable(mx::InputPtr input)
        : Variable{ModifierList{TokenType::Mutable}, std::make_shared<ResolvedTypeInfo>(input->getType())}, input_{std::move(input)}
    {

    }

protected:
    ValuePtr value_impl() const override
    {
        return ValueFactory::copy_value_from_port(input_);
    }

    void set_value_impl(const ValuePtr value) override
    {
        value->set_as_node_input(input_);
    }

private:
    mx::InputPtr input_;
};

#endif //MXSLC_INPUTVARIABLE_H
