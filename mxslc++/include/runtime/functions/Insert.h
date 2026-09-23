//
// Created by jaket on 19/09/2026.
//

#ifndef MXSLC_INSERT_H
#define MXSLC_INSERT_H

#include "runtime/functions/BuiltinFunction.h"

namespace mxslc::runtime
{
    class ArgumentList;

    class Insert : public BuiltinFunction
    {
    public:
        Insert(const VarPtr& instance, const ArgumentList& args);

        void init() override;
        TypePtr return_type() const override;
        VarPtr invoke() const override;
    };
}

#endif //MXSLC_INSERT_H
