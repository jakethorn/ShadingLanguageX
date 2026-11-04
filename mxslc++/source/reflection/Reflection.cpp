//
// Created by jaket on 04/11/2026.
//

#include "reflection/Reflection.h"

namespace mxslc::reflection
{
    void Reflection::add_global(string name, TypePtr type, VarPtr value)
    {
        // globals in different scopes are the same global
        for (const GlobalInfo& global : globals_)
            if (global.name == name)
                return;
        globals_.push_back(GlobalInfo{std::move(name), std::move(type), std::move(value)});
    }
}
