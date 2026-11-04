//
// Created by jaket on 04/11/2026.
//

#ifndef MXSLC_REFLECTION_H
#define MXSLC_REFLECTION_H

#include "common.h"
#include "GlobalInfo.h"

namespace mxslc::reflection
{
    class Reflection
    {
    public:
        void add_global(string name, TypePtr type, VarPtr value);
        const vector<GlobalInfo>& globals() const { return globals_; }

    private:

        vector<GlobalInfo> globals_;
    };
}

#endif //MXSLC_REFLECTION_H
