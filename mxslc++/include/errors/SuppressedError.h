//
// Created by jaket on 23/11/2026.
//

#ifndef MXSLC_SUPPRESSEDERROR_H
#define MXSLC_SUPPRESSEDERROR_H

#include "CompileError.h"

namespace mxslc
{
    class SuppressedError final : public CompileError
    {
    public:
        SuppressedError() : CompileError{vector<Diagnostic>{}} { }
    };
}

#endif //MXSLC_SUPPRESSEDERROR_H
