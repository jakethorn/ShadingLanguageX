//
// Created by jaket on 19/09/2026.
//

#ifndef MXSLC_EXTEND_H
#define MXSLC_EXTEND_H

#include "runtime/functions/BuiltinFunction.h"

namespace mxslc::runtime
{
    class ArgumentList;

    class Extend : public BuiltinFunction
    {
    public:
        Extend(const VarPtr& instance, const ArgumentList& args);

        void init() override;
        TypePtr return_type() const override;
        VarPtr invoke() const override;
    };
}

#endif //MXSLC_EXTEND_H
