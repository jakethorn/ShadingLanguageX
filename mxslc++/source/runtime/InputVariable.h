//
// Created by jaket on 03/05/2026.
//

#ifndef MXSLC_INPUTVARIABLE_H
#define MXSLC_INPUTVARIABLE_H

#include "utils/common.h"
#include "Variable.h"

class InputVariable final : public Variable
{
public:
    explicit InputVariable(mx::InputPtr input);

protected:
    ValuePtr value_impl() const override;
    void set_value_impl(ValuePtr value) override;

private:
    mx::InputPtr input_;
};

#endif //MXSLC_INPUTVARIABLE_H
