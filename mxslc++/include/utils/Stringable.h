//
// Created by jaket on 07/07/2026.
//

#ifndef MXSLC_STRINGABLE_H
#define MXSLC_STRINGABLE_H

#include "common.h"

namespace mxslc
{
    class Stringable
    {
    public:
        Stringable() = default;
        virtual ~Stringable() = default;

        virtual string str() const = 0;
    };
}

#endif //MXSLC_STRINGABLE_H
