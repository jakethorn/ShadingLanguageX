//
// Created by jaket on 27/06/2026.
//

#ifndef MXSLC_AMBIGUOUSFUNCTIONERROR_H
#define MXSLC_AMBIGUOUSFUNCTIONERROR_H

#include "common.h"

class AmbiguousFunctionError final : public CompileError
{
public:
    explicit AmbiguousFunctionError(const string& message);
};

#endif //MXSLC_AMBIGUOUSFUNCTIONERROR_H
