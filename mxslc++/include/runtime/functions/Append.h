//
// Created by jaket on 19/09/2026.
//

#ifndef MXSLC_APPEND_H
#define MXSLC_APPEND_H

#include "runtime/functions/BuiltinFunction.h"

namespace mxslc::runtime
{
    class ArgumentList;

    class Append : public BuiltinFunction
    {
    public:
        Append(const VarPtr& instance, const ArgumentList& args);

        void init() override;
        TypePtr return_type() const override;
        VarPtr invoke() const override;
    };
}

#endif //MXSLC_APPEND_H
