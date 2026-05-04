//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_PORTACCESSOR_H
#define MXSLC_PORTACCESSOR_H

#include "DotAccessor.h"
#include "utils/common.h"

class PortAccessor final : public DotAccessor
{
public:
    PortAccessor(VarPtr node_var, string input_name);

    TypePtr type() const override;
    VarPtr evaluate() const override;

private:
    VarPtr node_var_;
    string input_name_;
    mx::InputPtr input_;
};

#endif //MXSLC_PORTACCESSOR_H
