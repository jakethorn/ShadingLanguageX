//
// Created by jaket on 23/11/2026.
//

#ifndef MXSLC_FATALERROR_H
#define MXSLC_FATALERROR_H

#include "CompileError.h"

namespace mxslc
{
    class FatalError final : public CompileError
    {
    public:
        using CompileError::CompileError;
    };
}

#endif //MXSLC_FATALERROR_H
