//
// Created by jaket on 11/04/2026.
//

#include "runtime/ModifierList.h"

#include "Token.h"
#include "errors/CompileError.h"

namespace mxslc::runtime
{
    ModifierList::ModifierList(const TokenType::Enum mod)
    {
        add(mod);
    }

    ModifierList::ModifierList(const vector<Token>& mods)
    {
        for (const Token& token : mods)
        {
            const TokenType mod = token.type();
            if (not contains(mod))
                add(mod);
            else
                throw CompileError{"Multiple '" + mod.to_string() + "' modifiers"};
        }
    }

    string ModifierList::to_string() const
    {
        return join(mods_, " ");
    }
}
