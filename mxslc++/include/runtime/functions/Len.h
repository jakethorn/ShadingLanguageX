//
// Created by jaket on 19/09/2026.
//

#ifndef MXSLC_LEN_H
#define MXSLC_LEN_H

#include "runtime/functions/BuiltinFunction.h"

namespace mxslc::runtime
{
    class ArgumentList;

    class Len : public BuiltinFunction
    {
    public:
        Len(const VarPtr& instance, const ArgumentList& args);

        TypePtr return_type() const override;
        VarPtr invoke() const override;
    };
}

#endif //MXSLC_LEN_H
