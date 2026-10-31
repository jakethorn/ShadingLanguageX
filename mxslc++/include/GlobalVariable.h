//
// Created by jaket on 30/10/2026.
//

#ifndef MXSLC_GLOBALVARIABLE_H
#define MXSLC_GLOBALVARIABLE_H

#include "common.h"

namespace mxslc
{
    // A `global` variable declared by an SLX program.
    struct GlobalVariable
    {
        string name;
        TypePtr type;
        // The value the variable had when the program ran. get_globals runs programs without global values, so
        // this is the initializer's value, or the type's default value if the declaration has no initializer.
        VarPtr value;
    };
}

#endif //MXSLC_GLOBALVARIABLE_H
