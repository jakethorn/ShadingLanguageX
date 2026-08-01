//
// Created by jaket on 04/05/2026.
//

#ifndef MXSLC_ACCESSOR_H
#define MXSLC_ACCESSOR_H

#include "common.h"
#include "runtime/utils/RuntimeAware.h"

namespace mxslc::expressions
{
    class Accessor : protected runtime_utils::RuntimeAware
    {
    public:
        Accessor() = default;
        ~Accessor() override = default;

        virtual TypePtr type() const = 0;
        virtual VarPtr evaluate() const = 0;
    };
}

#endif //MXSLC_ACCESSOR_H
